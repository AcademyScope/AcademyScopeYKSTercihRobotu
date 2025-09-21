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
    proxyUniversity->sort(0);  // Türkçe sıralama aktif

    auto *proxyDepartment = new TurkishFilterProxy(this);
    proxyDepartment->setSourceModel(ui->comboBoxDepartment->model());
    proxyDepartment->sort(0);  // Türkçe sıralama aktif

    auto *completerUniversity = new QCompleter(proxyUniversity, this);
    completerUniversity->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    // Case'i proxy yönettiği için:
    completerUniversity->setCaseSensitivity(Qt::CaseSensitive);

    auto *completerDepartment = new QCompleter(proxyDepartment, this);
    completerDepartment->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    // Case'i proxy yönettiği için:
    completerDepartment->setCaseSensitivity(Qt::CaseSensitive);

    ui->comboBoxUniversity->setEditable(true);
    ui->comboBoxUniversity->setCompleter(completerUniversity);

    ui->comboBoxDepartment->setEditable(true);
    ui->comboBoxDepartment->setCompleter(completerDepartment);

    // Kullanıcı yazdıkça filtre güncelle
    connect(ui->comboBoxUniversity->lineEdit(), &QLineEdit::textEdited,
            proxyUniversity, [proxyUniversity](const QString &t){ proxyUniversity->setNeedle(t); });

    connect(ui->comboBoxDepartment->lineEdit(), &QLineEdit::textEdited,
            proxyDepartment, [proxyDepartment](const QString &t){ proxyDepartment->setNeedle(t); });


    hideUnusedColumnsOnTheProgramTable();
    hideUnnecessaryColumnsOnTheProgramTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBoxUniversity_editTextChanged(const QString &arg1)
{
    populateProgramTable();
    //ui->comboBoxUniversite->showPopup();
}

/*
void MainWindow::populateUniversitiesComboBox()
{

    // Model üzerinden ilk item'a eriş
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboBoxUniversite->model());
    QStandardItem* firstItem = model->item(0);

    // Disable + gri göster
    firstItem->setFlags(firstItem->flags() & ~Qt::ItemIsEnabled);
    firstItem->setForeground(QBrush(Qt::gray));


    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Volumes/Projects/YKSEpiSelecta/YKS.SQLite");

    if (!db.open()) {
        qDebug() << "Veritabanı açılamadı:" << db.lastError().text();
    } else {
        qDebug() << "Veritabanına bağlanıldı.";
    }


    QSqlQuery query;

    // Veri okuma
    if (query.exec("SELECT UniversiteID, UniversiteAdi FROM Universiteler")) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString universiteAdi = query.value(1).toString();
            qDebug() << id << universiteAdi;
            ui->comboBoxUniversite->addItem(universiteAdi);
        }
    }

}
*/

void MainWindow::initDB() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Volumes/Projects/YKSEpiSelecta/YKS.SQLite");

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

    // Türkçe collator ile sırala
    QCollator collator(QLocale(QLocale::Turkish, QLocale::Turkey));
    std::sort(departments.begin(), departments.end(),
              [&](const QString &a, const QString &b) {
                  return collator.compare(a, b) < 0;
              });

    // ComboBox’a ekle
    for (auto &department : departments) {
        ui->comboBoxDepartment->addItem(department);
    }
    ui->comboBoxDepartment->clearEditText();
}


void MainWindow::populateProgramTable(){
    QString universityName = ui->comboBoxUniversity->currentText();
    universityName = turkishLocale.toUpper(universityName);
    QSqlQuery query;
    QList<QPair<int, QString>> universities;

    if (query.exec("SELECT * FROM YKS WHERE UniversiteAdi LIKE \"%" + universityName + "%\"")) {
        ui->tableWidgetPrograms->setRowCount(0);
        hideUnnecessaryColumnsOnTheProgramTable();

        int row = 0;
        while (query.next()) {
            ui->tableWidgetPrograms->insertRow(row);
            int id = query.value("ProgramKodu").toInt();
            QString universityType = query.value("UniversiteTuru").toString();
            QString universityName = query.value("UniversiteAdi").toString();
            QString facultyName = query.value("FakulteYuksekokulAdi").toString();
            QString programName = query.value("ProgramAdi").toString();
            QString pointsKind = query.value("PuanTuru").toString();
            int genelKontenjan = query.value("Genel_Kontenjan").toInt();
            int genelYerlesen = query.value("Genel_Yerlesen").toInt();
            double generalMinimumPoints = query.value("Genel_EnKucukPuan").toDouble();
            double generalMaximumPoints = query.value("Genel_EnBuyukPuan").toDouble();

            QString okulBirincisiKontenjan = query.value("OB_Kontenjan").toString();
            QString okulBirincisiEnKucukPuan = query.value("OB_EnKucukPuan").toString();
            QString okulBirincisiYerlesen = query.value("OB_Yerlesen").toString();

            QString sehitGaziKontenjan = query.value("SehitGazi_Kontenjan").toString();
            QString sehitGaziEnKucukPuan = query.value("SehitGazi_EnKucukPuan").toString();
            QString sehitGaziYerlesen = query.value("SehitGazi_Yerlesen").toString();

            QString depremzedeKontenjan = query.value("Dep_Kontenjan").toString();
            QString depremzedeEnKucukPuan = query.value("Dep_EnKucukPuan").toString();
            QString depremzedeYerlesen = query.value("Dep_Yerlesen").toString();

            QString kadin34Kontenjan = query.value("Kadin34_Kontenjan").toString();
            QString kadin34EnKucukPuan = query.value("Kadin34_EnKucukPuan").toString();
            QString kadin34Yerlesen = query.value("Kadin34_Yerlesen").toString();

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::ProgramKodu, new QTableWidgetItem(QString::number(id)));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Universite, new QTableWidgetItem(universityName));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kampus, new QTableWidgetItem(facultyName));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Program, new QTableWidgetItem(programName));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::PuanTuru, new QTableWidgetItem(pointsKind));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelKontenjan, new QTableWidgetItem(QString::number(genelKontenjan)));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelYerlesen, new QTableWidgetItem(QString::number(genelYerlesen)));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::GenelEnKucukPuan, new QTableWidgetItem(QString::number(generalMinimumPoints)));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiKontenjan, new QTableWidgetItem(okulBirincisiKontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiYerlesen, new QTableWidgetItem(okulBirincisiYerlesen));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::OkulBirincisiEnKucukPuan, new QTableWidgetItem(okulBirincisiEnKucukPuan));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniKontenjan, new QTableWidgetItem(sehitGaziKontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniYerlesen, new QTableWidgetItem(sehitGaziYerlesen));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::SehitGaziYakiniEnKucukPuan, new QTableWidgetItem(sehitGaziEnKucukPuan));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeKontenjan, new QTableWidgetItem(depremzedeKontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeYerlesen, new QTableWidgetItem(depremzedeYerlesen));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::DepremzedeEnKucukPuan, new QTableWidgetItem(depremzedeEnKucukPuan));

            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusKontenjan, new QTableWidgetItem(kadin34Kontenjan));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusYerlesen, new QTableWidgetItem(kadin34Yerlesen));
            ui->tableWidgetPrograms->setItem(row, (int) ProgramTableColumns::Kadin34PlusEnKucukPuan, new QTableWidgetItem(kadin34EnKucukPuan));

            row++;
            //qDebug()<< "Program adı: " << programName;
        }
    }

    return;

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
}

void MainWindow::hideUnnecessaryColumnsOnTheProgramTable() {
    if(ui->checkBoxGenel->isChecked()) {
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


}

void MainWindow::hideUnusedColumnsOnTheProgramTable() {
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::GenelBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::OkulBirincisiBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::SehitGaziYakiniBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::DepremzedeBasariSirasi);
    ui->tableWidgetPrograms->hideColumn((int) ProgramTableColumns::Kadin34PlusBasariSirasi);
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

}

