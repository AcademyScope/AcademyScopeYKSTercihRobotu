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
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::ProgramKodu, 100);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::Universite, 300);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::Kampus, 170);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::Program, 300);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::PuanTuru, 40);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::GenelKontenjan, 60);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::GenelYerlesen, 60);
    ui->tableWidgetPrograms->setColumnWidth((int) ProgramTableColumns::GenelEnKucukPuan, 100);
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


void MainWindow::populateProgramTable(){
    ui->tableWidgetPrograms->clearContents();
    ui->tableWidgetPrograms->setRowCount(0);

    if (!db.isOpen())
        return;

    hideUnnecessaryColumnsOnTheProgramTable();

    ui->tableWidgetPrograms->setUpdatesEnabled(false);
    ui->tableWidgetPrograms->setSortingEnabled(false);

    QString universityName = ui->comboBoxUniversity->currentText();
    QString department = ui->comboBoxDepartment->currentText();
    universityName = turkishLocale.toUpper(universityName);
    QSqlQuery query;

    QStringList whereQueries;
    QStringList kontenjanQueries;
    QStringList tuitionQueries;
    QStringList gradeIntervalQueries;
    QString kontenjanSubQuery = "";
    QString tuitionSubQuery = "";
    QString gradeIntervalSubQuery = "";
    QList<QPair<int, QString>> universities;

    QString sqlQuery = "Select * FROM ";
    if(tercihTuru == TercihTuru::NormalTercih)
        sqlQuery += "YKS";
    else if(tercihTuru == TercihTuru::EkTercih)
        sqlQuery += "EkTercihDetayli";

    if(universityName.trimmed() != "") {
        whereQueries.append("UniversiteAdi LIKE \"%" + StringUtil::toTurkishUpperCase(universityName) + "%\"");
    }

    if(department.trimmed() != "") {
        whereQueries.append("ProgramAdi LIKE \"%" + StringUtil::toTurkishTitleCase(department) + "%\"");
    }

    int ulkeIndex = ui->comboBoxUlke->currentIndex();
    if(ulkeIndex == 1) { //Turkiye
        whereQueries.append("UlkeKodu = 90");
    }
    else if(ulkeIndex == 2) { //Kibris
        whereQueries.append("UlkeKodu = 357");
    }
    else if(ulkeIndex == 3) { //Yurtdisi
        whereQueries.append("UlkeKodu <> 90");
        whereQueries.append("UlkeKodu <> 357");
    }

    if(ui->comboBoxLicenseType->currentIndex() == 1) {
        whereQueries.append("Lisans = 1");
    }
    else if(ui->comboBoxLicenseType->currentIndex() == 2) {
        whereQueries.append("Lisans = 0");
    }

    if(ui->comboBoxUniversityType->currentIndex() == 1) { // Devlet
        whereQueries.append("DevletUniversitesi = 1");
    }
    else if(ui->comboBoxUniversityType->currentIndex() == 2) { //Vakıf
        whereQueries.append("DevletUniversitesi = 0");
    }

    int puanTuruIndex = ui->comboBoxPuanTuru->currentIndex();
    switch(puanTuruIndex) {
    case 1:
        whereQueries.append("PuanTuru = \"SAY\"");
        break;
    case 2:
        whereQueries.append("PuanTuru = \"EA\"");
        break;
    case 3:
        whereQueries.append("PuanTuru = \"SÖZ\"");
        break;
    case 4:
        whereQueries.append("PuanTuru = \"TYT\"");
        break;
    case 5:
        whereQueries.append("PuanTuru = \"DİL\"");
        break;
    }

    double enKucukPuan = ui->doubleSpinBoxEnKucukPuan->value();
    double enBuyukPuan = ui->doubleSpinBoxEnBuyukPuan->value();

    QString genelPuanAraligiQuery = "";
    if(ui->checkBoxGenel->isChecked() || ui->checkBoxKKTCUyruklu->isChecked() || ui->checkBoxMTOK->isChecked()) {
        if(enKucukPuan > 100) {
            genelPuanAraligiQuery = "GenelEnKucukPuan > " + QString::number(enKucukPuan);
        }
        if(enBuyukPuan < 560) {
            if(!genelPuanAraligiQuery.isEmpty())
                genelPuanAraligiQuery += " AND ";
            genelPuanAraligiQuery += "GenelEnKucukPuan < " + QString::number(enBuyukPuan);
        }
        if(!genelPuanAraligiQuery.isEmpty())
            gradeIntervalQueries.append(genelPuanAraligiQuery);
    }

    QString okulBirincisiPuanAraligiQuery = "";
    if(ui->checkBoxOkulBirincisi->isChecked()) {
        if(enKucukPuan > 100) {
            okulBirincisiPuanAraligiQuery = "OkulBirincisiEnKucukPuan > " + QString::number(enKucukPuan);
        }
        if(enBuyukPuan < 560) {
            if(!okulBirincisiPuanAraligiQuery.isEmpty())
                okulBirincisiPuanAraligiQuery += " AND ";
            okulBirincisiPuanAraligiQuery += "OkulBirincisiEnKucukPuan < " + QString::number(enBuyukPuan);
        }
        if(!okulBirincisiPuanAraligiQuery.isEmpty())
            gradeIntervalQueries.append(okulBirincisiPuanAraligiQuery);
    }

    QString sehitGaziPuanAraligiQuery = "";
    if(ui->checkBoxSehitGaziYakini->isChecked()) {
        if(enKucukPuan > 100) {
            sehitGaziPuanAraligiQuery = "SehitGaziEnKucukPuan > " + QString::number(enKucukPuan);
        }
        if(enBuyukPuan < 560) {
            if(!sehitGaziPuanAraligiQuery.isEmpty())
                sehitGaziPuanAraligiQuery += " AND ";
            sehitGaziPuanAraligiQuery += "SehitGaziEnKucukPuan < " + QString::number(enBuyukPuan);
        }
        if(!sehitGaziPuanAraligiQuery.isEmpty())
            gradeIntervalQueries.append(sehitGaziPuanAraligiQuery);
    }

    QString depremzedePuanAraligiQuery = "";
    if(ui->checkBoxDepremzede->isChecked()) {
        if(enKucukPuan > 100) {
            depremzedePuanAraligiQuery = "DepremzedeEnKucukPuan > " + QString::number(enKucukPuan);
        }
        if(enBuyukPuan < 560) {
            if(!depremzedePuanAraligiQuery.isEmpty())
                depremzedePuanAraligiQuery += " AND ";
            depremzedePuanAraligiQuery += "DepremzedeEnKucukPuan < " + QString::number(enBuyukPuan);
        }
        if(!depremzedePuanAraligiQuery.isEmpty())
            gradeIntervalQueries.append(depremzedePuanAraligiQuery);
    }

    QString kadin34PuanAraligiQuery = "";
    if(ui->checkBoxKadin34->isChecked()) {
        if(enKucukPuan > 100) {
            kadin34PuanAraligiQuery = "Kadin34EnKucukPuan > " + QString::number(enKucukPuan);
        }
        if(enBuyukPuan < 560) {
            if(!kadin34PuanAraligiQuery.isEmpty())
                kadin34PuanAraligiQuery += " AND ";
            kadin34PuanAraligiQuery += "Kadin34EnKucukPuan < " + QString::number(enBuyukPuan);
        }
        if(!kadin34PuanAraligiQuery.isEmpty())
            gradeIntervalQueries.append(kadin34PuanAraligiQuery);
    }

    if(!gradeIntervalQueries.isEmpty()) {
        gradeIntervalSubQuery += "(" + gradeIntervalQueries[0];
        for(int i = 1; i < gradeIntervalQueries.size(); i++) {
            gradeIntervalSubQuery += " OR " + gradeIntervalQueries[i];
        }
        gradeIntervalSubQuery += ")";
    }

    if(!gradeIntervalSubQuery.isEmpty()) {
        whereQueries.append(gradeIntervalSubQuery);
    }

    if (ui->checkBoxGenel->isChecked()) {
        kontenjanQueries.append("GenelKontenjan IS NOT NULL");
    }

    if (ui->checkBoxOkulBirincisi->isChecked()) {
        kontenjanQueries.append("OkulBirincisiKontenjan IS NOT NULL");
    }

    if (ui->checkBoxSehitGaziYakini->isChecked()) {
        kontenjanQueries.append("SehitGaziKontenjan IS NOT NULL");
    }

    if (ui->checkBoxDepremzede->isChecked()) {
        kontenjanQueries.append("DepremzedeKontenjan IS NOT NULL");
    }

    if (ui->checkBoxKadin34->isChecked()) {
        kontenjanQueries.append("Kadin34Kontenjan IS NOT NULL");
    }

    if (ui->checkBoxUcretsiz->isChecked()) {
        tuitionQueries.append("UcretDurumu = 0");
    }

    if (ui->checkBoxIndirimli->isChecked()) {
        tuitionQueries.append("UcretDurumu = 50");
    }

    if (ui->checkBoxUcretli->isChecked()) {
        tuitionQueries.append("UcretDurumu = 100");
    }

    if (ui->checkBoxKKTCUyruklu->isChecked()) {
        kontenjanQueries.append("KKTCUyruklu = TRUE"); //In order to add OR Query, it is appended to kontenjanQueries
    }
    else {
        whereQueries.append("KKTCUyruklu = FALSE"); //In order to add AND Query, it is appended to whereQueries
    }

    if (ui->checkBoxMTOK->isChecked()) {
        kontenjanQueries.append("MTOK = TRUE"); //In order to add OR Query, it is appended to kontenjanQueries
    }
    else {
        whereQueries.append("MTOK = FALSE"); //In order to add AND Query, it is appended to whereQueries
    }

    if(!kontenjanQueries.isEmpty()) {
        kontenjanSubQuery += "(" + kontenjanQueries[0];
        for(int i = 1; i < kontenjanQueries.size(); i++) {
            kontenjanSubQuery += " OR " + kontenjanQueries[i];
        }
        kontenjanSubQuery += ")";
    }

    if(!kontenjanSubQuery.isEmpty()) {
        whereQueries.append(kontenjanSubQuery);
    }

    if(!tuitionQueries.isEmpty()) {
        tuitionSubQuery += "(" + tuitionQueries[0];
        for(int i = 1; i < tuitionQueries.size(); i++) {
            tuitionSubQuery += " OR " + tuitionQueries[i];
        }
        tuitionSubQuery += ")";
    }

    if(!tuitionSubQuery.isEmpty())
        whereQueries.append(tuitionSubQuery);

    if(kontenjanSubQuery.isEmpty() || tuitionSubQuery.isEmpty()) {
        ui->tableWidgetPrograms->setRowCount(0);
        ui->tableWidgetPrograms->setUpdatesEnabled(true);
        ui->tableWidgetPrograms->setSortingEnabled(true);
        return;
    }

    // LASTLY (FINALLY) process "WHERE" queries
    if(!whereQueries.isEmpty()) {
        sqlQuery += " WHERE ";
        sqlQuery += whereQueries[0];
        for(int i = 1; i < whereQueries.size(); i++) {
            sqlQuery += " AND " + whereQueries[i];
        }
    }

    if(lastSortCol == -1) {
        const QString key = SQLiteUtil::trOrderExprFor("ProgramKodu");
        sqlQuery += " ORDER BY ProgramKodu ASC";
    }
    else if(!ui->tableWidgetPrograms->isColumnHidden(lastSortCol)) {
        QString col = getDbColumnNameFromProgramTableColumnIndex(lastSortCol);
        const QString key = SQLiteUtil::trOrderExprFor(col);
        sqlQuery += " ORDER BY " + key;
        if(lastSortOrder == Qt::AscendingOrder)
            sqlQuery += " ASC";
        else
            sqlQuery += " DESC";
    }

    if (query.exec(sqlQuery)) {
        int row = 0;
        while (query.next()) {
            ui->tableWidgetPrograms->insertRow(row);
            int id = query.value("ProgramKodu").toInt();
            QString universityType = query.value("UniversiteTuru").toString();
            QString universityName = query.value("UniversiteAdi").toString();
            QString facultyName = query.value("FakulteYuksekokulAdi").toString();
            QString programAdi = query.value("ProgramAdi").toString();
            QString puanTuru = query.value("PuanTuru").toString();
            QString genelKontenjan = query.value("GenelKontenjan").toString();
            QString generalMinimumPoints = query.value("GenelEnKucukPuan").toString();

            QString sehitGaziKontenjan = query.value("SehitGaziKontenjan").toString();
            QString sehitGaziEnKucukPuan = query.value("SehitGaziEnKucukPuan").toString();

            QString depremzedeKontenjan = query.value("DepremzedeKontenjan").toString();
            QString depremzedeEnKucukPuan = query.value("DepremzedeEnKucukPuan").toString();

            QString kadin34Kontenjan = query.value("Kadin34Kontenjan").toString();
            QString kadin34EnKucukPuan = query.value("Kadin34EnKucukPuan").toString();

            //Ek kontenjanda yok
            if(tercihTuru == TercihTuru::NormalTercih) {
                //Ek kontenjanda yok
                QString okulBirincisiKontenjan = query.value("OkulBirincisiKontenjan").toString();
                QString okulBirincisiEnKucukPuan = query.value("OkulBirincisiEnKucukPuan").toString();

                QString okulBirincisiYerlesen = query.value("OkulBirincisiYerlesen").toString();
                QString sehitGaziYerlesen = query.value("SehitGaziYerlesen").toString();
                QString depremzedeYerlesen = query.value("DepremzedeYerlesen").toString();
                QString kadin34Yerlesen = query.value("Kadin34Yerlesen").toString();

                QString genelYerlesen = query.value("GenelYerlesen").toString();
                QString generalMaximumPoints = query.value("GenelEnBuyukPuan").toString();

                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelYerlesen, createTableWidgetItem(genelYerlesen, Qt::AlignHCenter));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiYerlesen, createTableWidgetItem(okulBirincisiYerlesen, Qt::AlignHCenter));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniYerlesen, createTableWidgetItem(sehitGaziYerlesen, Qt::AlignHCenter));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeYerlesen, createTableWidgetItem(depremzedeYerlesen, Qt::AlignHCenter));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusYerlesen, createTableWidgetItem(kadin34Yerlesen, Qt::AlignHCenter));

                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiKontenjan, createTableWidgetItem(okulBirincisiKontenjan, Qt::AlignHCenter));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiEnKucukPuan, createTableWidgetItem(okulBirincisiEnKucukPuan, Qt::AlignLeft));
            }

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::ProgramKodu, createTableWidgetItem(QString::number(id), Qt::AlignLeft));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Universite, createTableWidgetItem(universityName, Qt::AlignLeft));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kampus, createTableWidgetItem(facultyName, Qt::AlignLeft));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Program, createTableWidgetItem(programAdi, Qt::AlignLeft));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::PuanTuru, createTableWidgetItem(puanTuru, Qt::AlignHCenter));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelKontenjan, createTableWidgetItem(genelKontenjan, Qt::AlignHCenter));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelEnKucukPuan, createTableWidgetItem(generalMinimumPoints, Qt::AlignLeft));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniKontenjan, createTableWidgetItem(sehitGaziKontenjan, Qt::AlignHCenter));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniEnKucukPuan, createTableWidgetItem(sehitGaziEnKucukPuan, Qt::AlignLeft));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeKontenjan, createTableWidgetItem(depremzedeKontenjan, Qt::AlignHCenter));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeEnKucukPuan, createTableWidgetItem(depremzedeEnKucukPuan, Qt::AlignLeft));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusKontenjan, createTableWidgetItem(kadin34Kontenjan, Qt::AlignHCenter));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusEnKucukPuan, createTableWidgetItem(kadin34EnKucukPuan, Qt::AlignLeft));

            row++;
        }
    }

    ui->tableWidgetPrograms->setUpdatesEnabled(true);
    ui->tableWidgetPrograms->setSortingEnabled(true);

    return;
}

void MainWindow::hideUnnecessaryColumnsOnTheProgramTable() {
    if(ui->checkBoxGenel->isChecked() || ui->checkBoxKKTCUyruklu->isChecked() || ui->checkBoxMTOK->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::GenelKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::GenelYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::GenelEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::GenelKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::GenelYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::GenelEnKucukPuan);
    }

    if(ui->checkBoxOkulBirincisi->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::OkulBirincisiKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::OkulBirincisiEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::OkulBirincisiKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::OkulBirincisiEnKucukPuan);
    }

    if(ui->checkBoxSehitGaziYakini->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::SehitGaziYakiniKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::SehitGaziYakiniEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::SehitGaziYakiniKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::SehitGaziYakiniEnKucukPuan);
    }


    if(ui->checkBoxDepremzede->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::DepremzedeKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::DepremzedeYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::DepremzedeEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::DepremzedeKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::DepremzedeYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::DepremzedeEnKucukPuan);
    }


    if(ui->checkBoxKadin34->isChecked()) {
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::Kadin34PlusKontenjan);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::Kadin34PlusYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::Kadin34PlusEnKucukPuan);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::Kadin34PlusKontenjan);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::Kadin34PlusYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::Kadin34PlusEnKucukPuan);
    }

    //Ek kontenjanda yok
    if(tercihTuru == TercihTuru::NormalTercih) {
        if(ui->checkBoxGenel->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::GenelYerlesen);
        if(ui->checkBoxOkulBirincisi->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::OkulBirincisiYerlesen);
        if(ui->checkBoxSehitGaziYakini->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::SehitGaziYakiniYerlesen);
        if(ui->checkBoxDepremzede->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::DepremzedeYerlesen);
        if(ui->checkBoxKadin34->isChecked())
            ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::Kadin34PlusYerlesen);
    }
    else {
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::GenelYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::DepremzedeYerlesen);
        ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::Kadin34PlusYerlesen);
    }
}

void MainWindow::hideUnusedColumnsOnTheProgramTable() {
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::GenelBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::OkulBirincisiBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::SehitGaziYakiniBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::DepremzedeBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::Kadin34PlusBasariSirasi);
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
    switch (static_cast<ProgramTableColumns>(columnIndex)) {
    case ProgramTableColumns::ProgramKodu:              return "ProgramKodu";
    case ProgramTableColumns::Universite:               return "UniversiteAdi";
    case ProgramTableColumns::Kampus:                   return "FakulteYuksekokulAdi";
    case ProgramTableColumns::Program:                  return "ProgramAdi";
    case ProgramTableColumns::PuanTuru:                 return "PuanTuru";
    case ProgramTableColumns::GenelKontenjan:           return "GenelKontenjan";
    case ProgramTableColumns::GenelYerlesen:            return "GenelYerlesen";
    case ProgramTableColumns::GenelEnKucukPuan:         return "GenelEnKucukPuan";
    case ProgramTableColumns::OkulBirincisiKontenjan:   return "OkulBirincisiKontenjan";
    case ProgramTableColumns::OkulBirincisiYerlesen:    return "OkulBirincisiYerlesen";
    case ProgramTableColumns::OkulBirincisiEnKucukPuan: return "OkulBirincisiEnKucukPuan";
    case ProgramTableColumns::SehitGaziYakiniKontenjan: return "SehitGaziKontenjan";
    case ProgramTableColumns::SehitGaziYakiniYerlesen:  return "SehitGaziYerlesen";
    case ProgramTableColumns::SehitGaziYakiniEnKucukPuan:return "SehitGaziEnKucukPuan";
    case ProgramTableColumns::DepremzedeKontenjan:      return "DepremzedeKontenjan";
    case ProgramTableColumns::DepremzedeYerlesen:       return "DepremzedeYerlesen";
    case ProgramTableColumns::DepremzedeEnKucukPuan:    return "DepremzedeEnKucukPuan";
    case ProgramTableColumns::Kadin34PlusKontenjan:     return "Kadin34Kontenjan";
    case ProgramTableColumns::Kadin34PlusYerlesen:      return "Kadin34Yerlesen";
    case ProgramTableColumns::Kadin34PlusEnKucukPuan:   return "Kadin34EnKucukPuan";
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

