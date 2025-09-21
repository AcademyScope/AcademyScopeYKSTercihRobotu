#pragma once

#include <QMainWindow>
#include <QLocale>

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

private:
    Ui::MainWindow *ui;
    void initDB();
    void setProgramTableColumnWidths();
    void populateUniversitiesComboBox();
    void populateDepartmentsComboBox();
    void populateProgramTable();
    void hideUnnecessaryColumnsOnTheProgramTable();
    void hideUnusedColumnsOnTheProgramTable();

    QLocale turkishLocale;
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
