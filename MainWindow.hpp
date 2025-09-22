#pragma once

#include <QMainWindow>
#include <QLocale>
#include <QSqlDatabase>

#define DB_PATH "/Volumes/Projects/YKSEpiSelecta/YKS.SQLite"

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

private:
    Ui::MainWindow *ui;
    void initDB();
    void setProgramTableColumnWidths();
    void populateUniversitiesComboBox();
    void populateDepartmentsComboBox();
    void populateProgramTable(int sortCol = -1, Qt::SortOrder ord = Qt::AscendingOrder);
    void hideUnnecessaryColumnsOnTheProgramTable();
    void hideUnusedColumnsOnTheProgramTable();
    void initializeYKSTableColumnNames();

    QLocale turkishLocale;
    int lastSortCol = -1;
    Qt::SortOrder lastSortOrder = Qt::AscendingOrder;
    QStringList yksTableColumnNames;
    QSqlDatabase db;
};

enum class ProgramTableColumns : int {
    ProgramKodu = 0,
    Universite,
    Kampus,
    Program,
    PuanTuru,
    GenelKontenjan,
    GenelYerlesen,
    GenelBasariSirasi,
    GenelEnKucukPuan,
    OkulBirincisiKontenjan,
    OkulBirincisiYerlesen,
    OkulBirincisiBasariSirasi,
    OkulBirincisiEnKucukPuan,
    SehitGaziYakiniKontenjan,
    SehitGaziYakiniYerlesen,
    SehitGaziYakiniBasariSirasi,
    SehitGaziYakiniEnKucukPuan,
    DepremzedeKontenjan,
    DepremzedeYerlesen,
    DepremzedeBasariSirasi,
    DepremzedeEnKucukPuan,
    Kadin34PlusKontenjan,
    Kadin34PlusYerlesen,
    Kadin34PlusBasariSirasi,
    Kadin34PlusEnKucukPuan
};
