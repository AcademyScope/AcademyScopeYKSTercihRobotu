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
#include <QScrollBar>
#include "Utils/DarkModeUtil.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , turkishLocale(QLocale::Turkish, QLocale::Turkey)
{
    ui->setupUi(this);
    backEnd = new AcademyScopeBackEnd();
    setLogoDarkMode(DarkModeUtil::isDarkMode());
    ui->doubleSpinBoxEnKucukPuan->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->doubleSpinBoxEnBuyukPuan->setButtonSymbols(QAbstractSpinBox::NoButtons);
    setProgramTableColumnWidths();
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

    connect(backEnd->getDataModel(), &AcademyScopeModel::columnVisibilityChanged, this, [this](int col, bool show){
        qDebug()<<"Hidden";
        ui->programTable->setColumnHidden(col, !show);
    });

    connect(ui->programTable->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onProgramTableScroll);

    hideUnusedColumnsOnTheProgramTable();
    hideUnnecessaryColumnsOnTheProgramTable();
    //backEnd->populateProgramTable(parameters);

    programTableHorizontalHeader = ui->programTable->horizontalHeader();
    programTableHorizontalHeader->setSortIndicatorShown(true);
    connect(programTableHorizontalHeader, &QHeaderView::sectionClicked, this, &MainWindow::onProgramTableHeaderItemClicked);
    ui->programTable->setModel(backEnd->getDataModel());
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
    parameters.order.toBeOrdered = true;
    parameters.order.column = (ProgramTableColumn) lastSortCol;
    parameters.order.direction = lastSortOrder;
    backEnd->populateProgramTable(parameters);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBoxUniversity_editTextChanged(const QString &arg1)
{
    parameters.universityName = arg1;
    backEnd->populateProgramTable(parameters);
}

void MainWindow::setProgramTableColumnWidths() {
    ui->programTable->setColumnWidth((int) ProgramTableColumn::ProgramKodu, 100);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::Universite, 300);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::Kampus, 170);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::Program, 300);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::PuanTuru, 40);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::GenelKontenjan, 60);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::GenelYerlesen, 60);
    ui->programTable->setColumnWidth((int) ProgramTableColumn::GenelEnKucukPuan, 100);
}

void MainWindow::populateUniversitiesComboBox() {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboBoxUniversity->model());
    QList<University> universities = backEnd->getUniversities();

    for (auto &u : universities) {
        ui->comboBoxUniversity->addItem(u.name, u.id);
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
    QList<QString> departments = backEnd->getDepartments();

    for (auto &department : departments) {
        ui->comboBoxDepartment->addItem(department);
    }
    ui->comboBoxDepartment->clearEditText();
}

void MainWindow::hideUnnecessaryColumnsOnTheProgramTable() {
    if(ui->checkBoxGenel->isChecked() || ui->checkBoxKKTCUyruklu->isChecked() || ui->checkBoxMTOK->isChecked()) {
        ui->programTable->showColumn((int) ProgramTableColumn::GenelKontenjan);
        ui->programTable->showColumn((int) ProgramTableColumn::GenelYerlesen);
        ui->programTable->showColumn((int) ProgramTableColumn::GenelEnKucukPuan);
    }
    else {
        ui->programTable->hideColumn((int) ProgramTableColumn::GenelKontenjan);
        ui->programTable->hideColumn((int) ProgramTableColumn::GenelYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::GenelEnKucukPuan);
    }

    if(ui->checkBoxOkulBirincisi->isChecked()) {
        ui->programTable->showColumn((int) ProgramTableColumn::OkulBirincisiKontenjan);
        ui->programTable->showColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        ui->programTable->showColumn((int) ProgramTableColumn::OkulBirincisiEnKucukPuan);
    }
    else {
        ui->programTable->hideColumn((int) ProgramTableColumn::OkulBirincisiKontenjan);
        ui->programTable->hideColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::OkulBirincisiEnKucukPuan);
    }

    if(ui->checkBoxSehitGaziYakini->isChecked()) {
        ui->programTable->showColumn((int) ProgramTableColumn::SehitGaziYakiniKontenjan);
        ui->programTable->showColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        ui->programTable->showColumn((int) ProgramTableColumn::SehitGaziYakiniEnKucukPuan);
    }
    else {
        ui->programTable->hideColumn((int) ProgramTableColumn::SehitGaziYakiniKontenjan);
        ui->programTable->hideColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::SehitGaziYakiniEnKucukPuan);
    }


    if(ui->checkBoxDepremzede->isChecked()) {
        ui->programTable->showColumn((int) ProgramTableColumn::DepremzedeKontenjan);
        ui->programTable->showColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        ui->programTable->showColumn((int) ProgramTableColumn::DepremzedeEnKucukPuan);
    }
    else {
        ui->programTable->hideColumn((int) ProgramTableColumn::DepremzedeKontenjan);
        ui->programTable->hideColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::DepremzedeEnKucukPuan);
    }


    if(ui->checkBoxKadin34->isChecked()) {
        ui->programTable->showColumn((int) ProgramTableColumn::Kadin34PlusKontenjan);
        ui->programTable->showColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
        ui->programTable->showColumn((int) ProgramTableColumn::Kadin34PlusEnKucukPuan);
    }
    else {
        ui->programTable->hideColumn((int) ProgramTableColumn::Kadin34PlusKontenjan);
        ui->programTable->hideColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::Kadin34PlusEnKucukPuan);
    }

    //Ek kontenjanda yok
    if(parameters.placementType == PlacementType::Regular) {
        if(ui->checkBoxGenel->isChecked())
            ui->programTable->showColumn((int) ProgramTableColumn::GenelYerlesen);
        if(ui->checkBoxOkulBirincisi->isChecked())
            ui->programTable->showColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        if(ui->checkBoxSehitGaziYakini->isChecked())
            ui->programTable->showColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        if(ui->checkBoxDepremzede->isChecked())
            ui->programTable->showColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        if(ui->checkBoxKadin34->isChecked())
            ui->programTable->showColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
    }
    else {
        ui->programTable->hideColumn((int) ProgramTableColumn::GenelYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::OkulBirincisiYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::SehitGaziYakiniYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::DepremzedeYerlesen);
        ui->programTable->hideColumn((int) ProgramTableColumn::Kadin34PlusYerlesen);
    }
}

void MainWindow::hideUnusedColumnsOnTheProgramTable() {
    ui->programTable->hideColumn((int) ProgramTableColumn::GenelBasariSirasi);
    ui->programTable->hideColumn((int) ProgramTableColumn::OkulBirincisiBasariSirasi);
    ui->programTable->hideColumn((int) ProgramTableColumn::SehitGaziYakiniBasariSirasi);
    ui->programTable->hideColumn((int) ProgramTableColumn::DepremzedeBasariSirasi);
    ui->programTable->hideColumn((int) ProgramTableColumn::Kadin34PlusBasariSirasi);
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

void MainWindow::getProgramTableVisibleRowIndexes(int &visibleTopRow, int &visibleBottomRow)
{
    QRect visibleRect = ui->programTable->viewport()->rect();

    QModelIndex topIndex    = ui->programTable->indexAt(QPoint(0, visibleRect.top()));
    QModelIndex bottomIndex = ui->programTable->indexAt(QPoint(0, visibleRect.bottom()));

    if (!topIndex.isValid())
        topIndex = ui->programTable->model()->index(0, 0);

    if (!bottomIndex.isValid()) {
        int lastRow = ui->programTable->model()->rowCount() - 1;
        bottomIndex = ui->programTable->model()->index(lastRow, 0);
    }

    visibleTopRow = topIndex.row();
    visibleBottomRow = bottomIndex.row();

    return;
}

QTableWidgetItem *MainWindow::createTableWidgetItem(const QString &text, const Qt::Alignment &alignment)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    return item;
}

void MainWindow::on_checkBoxGenel_toggled(bool checked)
{
    parameters.selectedQuotaTypes.regularQuota = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxOkulBirincisi_toggled(bool checked)
{
    parameters.selectedQuotaTypes.highSchoolValedictoriansQuota = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxSehitGaziYakini_toggled(bool checked)
{
    parameters.selectedQuotaTypes.martyrsAndVeteransQuota = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxDepremzede_toggled(bool checked)
{
    parameters.selectedQuotaTypes.earthquakeVictimsQuota = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxKadin34_toggled(bool checked)
{
    parameters.selectedQuotaTypes.women34PlusQuota = checked;
    backEnd->populateProgramTable(parameters);
}

void MainWindow::on_comboBoxDepartment_editTextChanged(const QString &arg1)
{
    parameters.departmentName = arg1;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_comboBoxUlke_currentIndexChanged(int index)
{
    switch(index) {
    case 0:
        parameters.country = Country::AllCountries;
        break;
    case 1:
        parameters.country = Country::Turkiye;
        break;
    case 2:
        parameters.country = Country::Cyprus;
        break;
    case 3:
        parameters.country = Country::ForeignCountries;
        break;
    default:
        parameters.country = Country::AllCountries;
        break;
    }
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_comboBoxDegreeType_currentIndexChanged(int index)
{
    switch(index) {
    case 0:
        parameters.degreeType = DegreeType::All;
        break;
    case 1:
        parameters.degreeType = DegreeType::Bachelor;
        break;
    case 2:
        parameters.degreeType = DegreeType::Associate;
        break;
    }
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_comboBoxUniversityType_currentIndexChanged(int index)
{
    switch(index) {
    case 0:
        parameters.universityType = UniversityType::Undefined;
        break;
    case 1:
        parameters.universityType = UniversityType::Government;
        break;
    case 2:
        parameters.universityType = UniversityType::Private;
        break;
    }
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxUcretsiz_toggled(bool checked)
{
    parameters.selectedTuitionFeeTypes.free = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxIndirimli_toggled(bool checked)
{
    parameters.selectedTuitionFeeTypes.discounted = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxUcretli_toggled(bool checked)
{
    parameters.selectedTuitionFeeTypes.paid = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxKKTCUyruklu_toggled(bool checked)
{
    parameters.selectedQuotaTypes.trncNationalsQuota = checked;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_checkBoxMTOK_toggled(bool checked)
{
    parameters.selectedQuotaTypes.mtokQuota = checked;
    backEnd->populateProgramTable(parameters);
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
        parameters.placementType = PlacementType::Regular;
    else
        parameters.placementType = PlacementType::Additional;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_pushButtonClearPuanAraligi_clicked()
{
    ui->doubleSpinBoxEnKucukPuan->setValue(100.0);
    ui->doubleSpinBoxEnBuyukPuan->setValue(560.0);
}


void MainWindow::on_doubleSpinBoxEnKucukPuan_valueChanged(double arg1)
{
    parameters.scoreInterval.minimum = arg1;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_doubleSpinBoxEnBuyukPuan_valueChanged(double arg1)
{
    parameters.scoreInterval.maximum = arg1;
    backEnd->populateProgramTable(parameters);
}


void MainWindow::on_comboBoxPuanTuru_currentIndexChanged(int index)
{
    switch(index) {
    case 0:
        parameters.trackType = TrackType::Undefined;
    break;
    case 1:
        parameters.trackType = TrackType::Science;
        break;
    case 2:
        parameters.trackType = TrackType::EqualWeight;
        break;
    case 3:
        parameters.trackType = TrackType::Humanities;
        break;
    case 4:
        parameters.trackType = TrackType::TYT;
        break;
    case 5:
        parameters.trackType = TrackType::Language;
        break;
    default:
        parameters.trackType = TrackType::Undefined;
        break;
    }
    backEnd->populateProgramTable(parameters);
}

void MainWindow::onProgramTableScroll(int scrollValue)
{
    int visibleTopRow, visibleBottomRow;
    getProgramTableVisibleRowIndexes(visibleTopRow, visibleBottomRow);
    //qDebug()<<"Top"<<visibleTopRow<<"Bottom"<<visibleBottomRow;
    auto *model = backEnd->getDataModel();
    if (!model) return;

    const int rowHeight   = ui->programTable->rowHeight(0);
    const int visibleRows = ui->programTable->viewport()->height() / rowHeight;
    const int totalRows   = model->getDataWindow()->tableRowCount;

    //const int topRow    = scrollValue / rowHeight;
    //const int bottomRow = std::min(totalRows - 1, topRow + visibleRows);

    auto *dataWindow = model->getDataWindow();
    const int newBegin = std::max(0, visibleTopRow);
    const int newEnd   = std::min(totalRows - 1, visibleTopRow + dataWindow->windowSize);

    if(visibleTopRow <= dataWindow->beginningIndex) {//going upward
        //qDebug()<<"going upward, visibleTopRow"<<visibleTopRow<<"dataWindow beginning index" << dataWindow->beginningIndex;
        dataWindow->beginningIndex -= dataWindow->windowSize/2;
        if(dataWindow->beginningIndex < 0)
            dataWindow->beginningIndex = 0;
        dataWindow->endingIndex = dataWindow->beginningIndex + dataWindow->windowSize - 1;
        model->loadCurrentWindow();
    }
    else if(visibleBottomRow >= dataWindow->endingIndex) {//going downward
        //qDebug()<<"going downward, visibleBottomRow"<<visibleTopRow<<"dataWindow ending index" << dataWindow->endingIndex;
        dataWindow->beginningIndex += dataWindow->windowSize/2;
        dataWindow->endingIndex = dataWindow->beginningIndex + dataWindow->windowSize - 1;
        if(dataWindow->endingIndex > dataWindow->tableRowCount - 1) {
            dataWindow->endingIndex = dataWindow->tableRowCount - 1;
            dataWindow->beginningIndex = dataWindow->endingIndex - dataWindow->windowSize + 1;
        }
        model->loadCurrentWindow();
    }
    //data is upward of the visible area || data is downward of the visible area
    if(visibleTopRow > dataWindow->endingIndex || visibleBottomRow < dataWindow->beginningIndex){
        //qDebug() << "Jump";
        dataWindow->beginningIndex = visibleTopRow - dataWindow->windowSize/2;
        dataWindow->endingIndex = dataWindow->beginningIndex + dataWindow->windowSize - 1;
        model->loadCurrentWindow();
    }
    return;

    if (newBegin != dataWindow->beginningIndex || newEnd != dataWindow->endingIndex) {
        dataWindow->beginningIndex = newBegin;
        dataWindow->endingIndex = newEnd;
        model->loadCurrentWindow();
    }
}

