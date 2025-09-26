#pragma once

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void on_buttonSinavCraft_clicked(QAbstractButton *button);

    void on_pushButton_clicked();

private:
    Ui::AboutDialog *ui;
    QFont *InterFont;
    QFont *InterFontTitle;
};
