/*
MainWindow class definitions of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "MainWindow.hpp"
#include "./ui_MainWindow.h"
#include <QCompleter>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardItemModel>
#include <QCollator>
#include "TurkishFilterProxy.hpp"
#include <QLineEdit>
#include <QCollator>
#include "AboutDialog.hpp"
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QtGlobal>
#include "Utils/SQLiteUtil.hpp"
#include "Utils/StringUtil.hpp"
#include "Utils/DarkModeUtil.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , turkishLocale(QLocale::Turkish, QLocale::Turkey)
{
    ui->setupUi(this);
    setLogoDarkMode(DarkModeUtil::isDarkMode());
    ui->doubleSpinBoxEnKucukPuan->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->doubleSpinBoxEnBuyukPuan->setButtonSymbols(QAbstractSpinBox::NoButtons);
    setProgramTableColumnWidths();
    initDB();
    populateUniversitiesComboBox();
    populateDepartmentsComboBox();

    auto *proxyUniversity = new TurkishFilterProxy(this);
    proxyUniversity->setSourceModel(ui->comboBoxUniversity->model());
    proxyUniversity->sort(0);

    auto *proxyDepartment = new TurkishFilterProxy(this);
    proxyDepartment->setSourceModel(ui->comboBoxDepartment->model());
    proxyDepartment->sort(0);

    auto *completerUniversity = new QCompleter(proxyUniversity, this);
    completerUniversity->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completerUniversity->setCaseSensitivity(Qt::CaseSensitive);

    auto *completerDepartment = new QCompleter(proxyDepartment, this);
    completerDepartment->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completerDepartment->setCaseSensitivity(Qt::CaseSensitive);

    ui->comboBoxUniversity->setEditable(true);
    ui->comboBoxUniversity->setCompleter(completerUniversity);

    ui->comboBoxDepartment->setEditable(true);
    ui->comboBoxDepartment->setCompleter(completerDepartment);

    connect(ui->comboBoxUniversity->lineEdit(), &QLineEdit::textEdited,
            proxyUniversity, [proxyUniversity](const QString &t){ proxyUniversity->setNeedle(t); });

    connect(ui->comboBoxDepartment->lineEdit(), &QLineEdit::textEdited,
            proxyDepartment, [proxyDepartment](const QString &t){ proxyDepartment->setNeedle(t); });


    hideUnusedColumnsOnTheProgramTable();
    hideUnnecessaryColumnsOnTheProgramTable();
    populateProgramTable();

    programTableHorizontalHeader = ui->tableWidgetPrograms->horizontalHeader();
    programTableHorizontalHeader->setSortIndicatorShown(true);
    connect(programTableHorizontalHeader, &QHeaderView::sectionClicked, this, &MainWindow::onProgramTableHeaderItemClicked);
}

bool MainWindow::event(QEvent *e) {
    if (e->type() == QEvent::ApplicationPaletteChange ||
        e->type() == QEvent::ThemeChange) {
        setLogoDarkMode(DarkModeUtil::isDarkMode());
        //qDebug() << "Theme changed. Dark mode?" << isDarkMode();
    }
    return QWidget::event(e);
}


void MainWindow::onProgramTableHeaderItemClicked(int logicalIndex) {
    qDebug()<<"item clicked " << getDbColumnNameFromProgramTableColumnIndex(logicalIndex);
    // When clicked to the title of a row again, change the order direction
    if (lastSortCol == logicalIndex)
        lastSortOrder = (lastSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder
                                                              : Qt::AscendingOrder;
    else {
        lastSortCol = logicalIndex;
        lastSortOrder = Qt::AscendingOrder;
    }
    programTableHorizontalHeader->setSortIndicator(lastSortCol, lastSortOrder);
    populateProgramTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBoxUniversity_editTextChanged(const QString &arg1)
{
    populateProgramTable();
}

void MainWindow::initDB() {
    db = QSqlDatabase::addDatabase("QSQLITE");

    QString dbPath = SQLiteUtil::resolveDatabasePath();

    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Veritabanı açılamadı:" << db.lastError().text();
        return;
    }
}

void MainWindow::setProgramTableColumnWidths() {
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::ProgramKodu, 100);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::Universite, 300);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::Kampus, 170);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::Program, 300);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::PuanTuru, 40);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::GenelKontenjan, 60);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::GenelYerlesen, 60);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumn::GenelEnKucukPuan, 100);
}

void MainWindow::populateUniversitiesComboBox() {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboBoxUniversity->model());
    /*
    QStandardItem* firstItem = model->item(0);
    firstItem->setFlags(firstItem->flags() & ~Qt::ItemIsEnabled);
    firstItem->setForeground(QBrush(Qt::gray));
    */
    QSqlQuery query;
    QList<QPair<int, QString>> universities;

    if (query.exec("SELECT UniversiteID, UniversiteAdi FROM Universiteler")) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();
            universities.append(qMakePair(id, name));
        }
    }

    // Türkçe collator ile sırala
    QCollator collator(QLocale(QLocale::Turkish, QLocale::Turkey));
    std::sort(universities.begin(), universities.end(),
              [&](const QPair<int, QString> &a, const QPair<int, QString> &b) {
                  return collator.compare(a.second, b.second) < 0;
              });

    // ComboBox’a ekle
    for (auto &u : universities) {
        ui->comboBoxUniversity->addItem(u.second, u.first);
    }
    ui->comboBoxUniversity->clearEditText();
}

void MainWindow::populateDepartmentsComboBox() {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboBoxDepartment->model());
    /*
    QStandardItem* firstItem = model->item(0);
    firstItem->setFlags(firstItem->flags() & ~Qt::ItemIsEnabled);
    firstItem->setForeground(QBrush(Qt::gray));
    */
    QSqlQuery query;
    QList<QString> departments;

    QString selectionQuery = "SELECT DISTINCT\n\
        TRIM(\n\
            CASE\n\
                WHEN instr(ProgramAdi, '(') > 0\n\
            THEN substr(ProgramAdi, 1, instr(ProgramAdi, '(') - 1)\n\
            ELSE ProgramAdi\n\
                END\n\
            ) AS AnaProgramAdi\n\
            FROM YKS;\
           ";

    if (query.exec(selectionQuery)) {
        while (query.next()) {
            QString department = query.value(0).toString();
            departments.append(department);
        }
    }

    QCollator collator(QLocale(QLocale::Turkish, QLocale::Turkey));
    std::sort(departments.begin(), departments.end(),
              [&](const QString &a, const QString &b) {
                  return collator.compare(a, b) < 0;
              });

    for (auto &department : departments) {
        ui->comboBoxDepartment->addItem(department);
    }
    ui->comboBoxDepartment->clearEditText();
}

void MainWindow::hideUnnecessaryColumnsOnTheProgramTable() {
    if(ui->checkBoxGenel->isChecked() || ui->checkBoxKKTCUyruklu->isChecked() || ui->checkBoxMTOK->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::GenelKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::GenelYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::GenelEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::GenelKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::GenelYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::GenelEnKucukPuan);
    }

    if(ui->checkBoxOkulBirincisi->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::OkulBirincisiKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::OkulBirincisiEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::OkulBirincisiKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::OkulBirincisiEnKucukPuan);
    }

    if(ui->checkBoxSehitGaziYakini->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::SehitGaziYakiniKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::SehitGaziYakiniEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::SehitGaziYakiniKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::SehitGaziYakiniEnKucukPuan);
    }


    if(ui->checkBoxDepremzede->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::DepremzedeKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::DepremzedeEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::DepremzedeKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::DepremzedeEnKucukPuan);
    }


    if(ui->checkBoxKadin34->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::Kadin34PlusKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::Kadin34PlusEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::Kadin34PlusKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::Kadin34PlusEnKucukPuan);
    }

    //Ek kontenjanda yok
    if(tercihTuru == TercihTuru::NormalTercih) {
        if(ui->checkBoxGenel->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::GenelYerlesen);
        if(ui->checkBoxOkulBirincisi->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        if(ui->checkBoxSehitGaziYakini->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        if(ui->checkBoxDepremzede->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        if(ui->checkBoxKadin34->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::GenelYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
    }
}

void MainWindow::hideUnusedColumnsOnTheProgramTable() {
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::GenelBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::OkulBirincisiBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::SehitGaziYakiniBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::DepremzedeBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumn::Kadin34PlusBasariSirasi);
}

void MainWindow::initializeYKSTableColumnNames()
{
    yksTableColumnNames = {
        "ProgramKodu",
        "UniversiteTuru",
        "UniversiteAdi",
        "FakulteYuksekokulAdi",
        "ProgramAdi",
        "PuanTuru",
        "GenelKontenjan",
        "GenelYerlesen",
        "GenelEnKucukPuan",
        "GenelEnBuyukPuan",
        "OkulBirincisiKontenjan",
        "OkulBirincisiYerlesen",
        "OkulBirincisiEnKucukPuan",
        "OkulBirincisiEnBuyukPuan",
        "DepremzedeKontenjan",
        "DepremzedeYerlesen",
        "DepremzedeEnKucukPuan",
        "DepremzedeEnBuyukPuan",
        "Kadin34Kontenjan",
        "Kadin34Yerlesen",
        "Kadin34EnKucukPuan",
        "Kadin34EnBuyukPuan",
        "SehitGaziKontenjan",
        "SehitGaziYerlesen",
        "SehitGaziEnKucukPuan",
        "SehitGaziEnBuyukPuan",
        "Lisans",
        "UlkeKodu"
    };
}

void MainWindow::setLogoDarkMode(bool isDarkMode) {
    if(isDarkMode) {
        ui->logo->setStyleSheet("image: url(:/Resources/Images/AcademyScopeDarkMode.png);");
    }
    else {
        ui->logo->setStyleSheet("image: url(:/Resources/Images/AcademyScope.png);");
    }
}

QString MainWindow::getDbColumnNameFromProgramTableColumnIndex(int columnIndex) {
    switch (static_cast<ProgramTableColumn>(columnIndex)) {
    case ProgramTableColumn::ProgramKodu:              return "ProgramKodu";
    case ProgramTableColumn::Universite:               return "UniversiteAdi";
    case ProgramTableColumn::Kampus:                   return "FakulteYuksekokulAdi";
    case ProgramTableColumn::Program:                  return "ProgramAdi";
    case ProgramTableColumn::PuanTuru:                 return "PuanTuru";
    case ProgramTableColumn::GenelKontenjan:           return "GenelKontenjan";
    case ProgramTableColumn::GenelYerlesen:            return "GenelYerlesen";
    case ProgramTableColumn::GenelEnKucukPuan:         return "GenelEnKucukPuan";
    case ProgramTableColumn::OkulBirincisiKontenjan:   return "OkulBirincisiKontenjan";
    case ProgramTableColumn::OkulBirincisiYerlesen:    return "OkulBirincisiYerlesen";
    case ProgramTableColumn::OkulBirincisiEnKucukPuan: return "OkulBirincisiEnKucukPuan";
    case ProgramTableColumn::SehitGaziYakiniKontenjan: return "SehitGaziKontenjan";
    case ProgramTableColumn::SehitGaziYakiniYerlesen:  return "SehitGaziYerlesen";
    case ProgramTableColumn::SehitGaziYakiniEnKucukPuan:return "SehitGaziEnKucukPuan";
    case ProgramTableColumn::DepremzedeKontenjan:      return "DepremzedeKontenjan";
    case ProgramTableColumn::DepremzedeYerlesen:       return "DepremzedeYerlesen";
    case ProgramTableColumn::DepremzedeEnKucukPuan:    return "DepremzedeEnKucukPuan";
    case ProgramTableColumn::Kadin34PlusKontenjan:     return "Kadin34Kontenjan";
    case ProgramTableColumn::Kadin34PlusYerlesen:      return "Kadin34Yerlesen";
    case ProgramTableColumn::Kadin34PlusEnKucukPuan:   return "Kadin34EnKucukPuan";
    default: return QString();
    }

}

QTableWidgetItem *MainWindow::createTableWidgetItem(const QString &text, const Qt::Alignment &alignment)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    return item;
}

void MainWindow::on_checkBoxGenel_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxOkulBirincisi_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxSehitGaziYakini_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxDepremzede_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxKadin34_toggled(bool checked)
{
    populateProgramTable();
}

void MainWindow::on_comboBoxDepartment_editTextChanged(const QString &arg1)
{
    populateProgramTable();
}


void MainWindow::on_comboBoxUlke_currentIndexChanged(int index)
{
    populateProgramTable();
}


void MainWindow::on_comboBoxLicenseType_currentIndexChanged(int index)
{
    populateProgramTable();
}


void MainWindow::on_comboBoxUniversityType_currentIndexChanged(int index)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxUcretsiz_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxIndirimli_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxUcretli_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxKKTCUyruklu_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxMTOK_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_checkBoxEkKontenjan_toggled(bool checked)
{
    populateProgramTable();
}


void MainWindow::on_pushButtonClearUniversityComboBox_clicked()
{
    ui->comboBoxUniversity->clearEditText();
}


void MainWindow::on_pushButtonClearDepartmentComboBox_clicked()
{
    ui->comboBoxDepartment->clearEditText();
}


void MainWindow::on_pushButtonAbout_clicked()
{
    AboutDialog aboutDialog;
    aboutDialog.exec();
}

void MainWindow::on_comboBoxTercihTuru_currentIndexChanged(int index)
{
    if(index == 0)
        tercihTuru = TercihTuru::NormalTercih;
    else
        tercihTuru = TercihTuru::EkTercih;
    populateProgramTable();
}


void MainWindow::on_pushButtonClearPuanAraligi_clicked()
{
    ui->doubleSpinBoxEnKucukPuan->setValue(100.0);
    ui->doubleSpinBoxEnBuyukPuan->setValue(560.0);
}


void MainWindow::on_doubleSpinBoxEnKucukPuan_valueChanged(double arg1)
{
    populateProgramTable();
}


void MainWindow::on_doubleSpinBoxEnBuyukPuan_valueChanged(double arg1)
{
    populateProgramTable();
}


void MainWindow::on_comboBoxPuanTuru_currentIndexChanged(int index)
{
    populateProgramTable();
}

