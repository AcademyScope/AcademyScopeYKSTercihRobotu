#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QObject>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    InterFont = new QFont("Inter", QFont::Normal);
    InterFont->setPixelSize(17);
    InterFontTitle = new QFont("Inter", QFont::Normal);
    InterFontTitle->setPixelSize(37);
    ui->labelMailAddress->setFont(*InterFont);
    ui->labelName->setFont(*InterFont);
    ui->labelSinavCraft->setFont(*InterFont);
    ui->labelTitle->setFont(*InterFontTitle);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonSinavCraft_clicked(QAbstractButton *button)
{
    close();
}


void AboutDialog::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.youtube.com/@SınavCraft"));
}

