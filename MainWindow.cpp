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
#include "SQLiteUtil.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , turkishLocale(QLocale::Turkish, QLocale::Turkey)
{
    ui->setupUi(this);
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

    if (query.exec("SELECT Distinct ProgramAdi FROM YKS")) {
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
    if (!db.isOpen())
        return;

    hideUnnecessaryColumnsOnTheProgramTable();

    QString universityName = ui->comboBoxUniversity->currentText();
    QString department = ui->comboBoxDepartment->currentText();
    universityName = turkishLocale.toUpper(universityName);
    QSqlQuery query;

    QStringList whereQueries;
    QStringList kontenjanQueries;
    QStringList tuitionQueries;
    QString kontenjanSubQuery = "";
    QString tuitionSubQuery = "";
    QList<QPair<int, QString>> universities;

    QString sqlQuery = "Select * FROM ";
    if(tercihTuru == TercihTuru::NormalTercih)
        sqlQuery += "YKS";
    else if(tercihTuru == TercihTuru::EkTercih)
        sqlQuery += "EkTercihDetayli";

    if(universityName.trimmed() != "") {
        whereQueries.append("UniversiteAdi LIKE \"%" + universityName + "%\"");
    }

    if(department.trimmed() != "") {
        whereQueries.append("ProgramAdi LIKE \"%" + department + "%\"");
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

    if(lastSortCol != -1 && !ui->tableWidgetPrograms->isColumnHidden(lastSortCol)) {
        sqlQuery += " ORDER BY " + getDbColumnNameFromProgramTableColumnIndex(lastSortCol);
        if(lastSortOrder == Qt::AscendingOrder)
            sqlQuery += " ASC";
        else
            sqlQuery += " DESC";
    }

    if (query.exec(sqlQuery)) {
        ui->tableWidgetPrograms->clearContents();
        ui->tableWidgetPrograms->setRowCount(0);

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

                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelYerlesen, new QTableWidgetItem(genelYerlesen));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiYerlesen, new QTableWidgetItem(okulBirincisiYerlesen));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniYerlesen, new QTableWidgetItem(sehitGaziYerlesen));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeYerlesen, new QTableWidgetItem(depremzedeYerlesen));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusYerlesen, new QTableWidgetItem(kadin34Yerlesen));

                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiKontenjan, new QTableWidgetItem(okulBirincisiKontenjan));
                ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiEnKucukPuan, new QTableWidgetItem(okulBirincisiEnKucukPuan));
            }

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::ProgramKodu, new QTableWidgetItem(QString::number(id)));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Universite, new QTableWidgetItem(universityName));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kampus, new QTableWidgetItem(facultyName));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Program, new QTableWidgetItem(programAdi));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::PuanTuru, new QTableWidgetItem(puanTuru));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelKontenjan, new QTableWidgetItem(genelKontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelEnKucukPuan, new QTableWidgetItem(generalMinimumPoints));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniKontenjan, new QTableWidgetItem(sehitGaziKontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniEnKucukPuan, new QTableWidgetItem(sehitGaziEnKucukPuan));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeKontenjan, new QTableWidgetItem(depremzedeKontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeEnKucukPuan, new QTableWidgetItem(depremzedeEnKucukPuan));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusKontenjan, new QTableWidgetItem(kadin34Kontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusEnKucukPuan, new QTableWidgetItem(kadin34EnKucukPuan));

            row++;
        }
    }

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
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::GenelYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::OkulBirincisiYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::SehitGaziYakiniYerlesen);
        ui->tableWidgetPrograms->showColumn((int) ProgramTableColumns::DepremzedeYerlesen);
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

