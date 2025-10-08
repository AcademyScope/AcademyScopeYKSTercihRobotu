/*
MainWindow class declarations of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <QMainWindow>
#include <QLocale>
#include <QSqlDatabase>
#include "EnumDefinitions.hpp"
#include <QHeaderView>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_comboBoxUniversity_editTextChanged(const QString &arg1);

    void on_checkBoxGenel_toggled(bool checked);

    void on_checkBoxOkulBirincisi_toggled(bool checked);

    void on_checkBoxSehitGaziYakini_toggled(bool checked);

    void on_checkBoxDepremzede_toggled(bool checked);

    void on_checkBoxKadin34_toggled(bool checked);

    void on_comboBoxDepartment_editTextChanged(const QString &arg1);

    void on_comboBoxUlke_currentIndexChanged(int index);

    void on_comboBoxLicenseType_currentIndexChanged(int index);

    void on_comboBoxUniversityType_currentIndexChanged(int index);

    void on_checkBoxUcretsiz_toggled(bool checked);

    void on_checkBoxIndirimli_toggled(bool checked);

    void on_checkBoxUcretli_toggled(bool checked);

    void on_checkBoxKKTCUyruklu_toggled(bool checked);

    void on_checkBoxMTOK_toggled(bool checked);

    void on_checkBoxEkKontenjan_toggled(bool checked);

    void on_pushButtonClearUniversityComboBox_clicked();

    void on_pushButtonClearDepartmentComboBox_clicked();

    void on_pushButtonAbout_clicked();

    void on_comboBoxTercihTuru_currentIndexChanged(int index);

    void onProgramTableHeaderItemClicked(int logicalIndex);

    void on_pushButtonClearPuanAraligi_clicked();

    void on_doubleSpinBoxEnKucukPuan_valueChanged(double arg1);

    void on_doubleSpinBoxEnBuyukPuan_valueChanged(double arg1);

    void on_comboBoxPuanTuru_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    void initDB();
    void setProgramTableColumnWidths();
    void populateUniversitiesComboBox();
    void populateDepartmentsComboBox();
    void hideUnnecessaryColumnsOnTheProgramTable();
    void hideUnusedColumnsOnTheProgramTable();
    void initializeYKSTableColumnNames();
    bool event(QEvent *e) override;
    void setLogoDarkMode(bool isDarkMode);
    QString getDbColumnNameFromProgramTableColumnIndex(int columnIndex);

    QTableWidgetItem* createTableWidgetItem(const QString &text, const Qt::Alignment &alignment = Qt::AlignLeft | Qt::AlignVCenter);

    QLocale turkishLocale;
    int lastSortCol = -1;
    Qt::SortOrder lastSortOrder = Qt::AscendingOrder;
    QHeaderView * programTableHorizontalHeader = nullptr;
    QStringList yksTableColumnNames;
    QSqlDatabase db;
    TercihTuru tercihTuru = TercihTuru::NormalTercih;
};
