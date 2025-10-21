/*
AboutDialog class definitions of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QObject>
#include "Utils/DarkModeUtil.hpp"

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

bool AboutDialog::event(QEvent *e) {
    if (e->type() == QEvent::ApplicationPaletteChange ||
        e->type() == QEvent::ThemeChange) {
        setLogoDarkMode(DarkModeUtil::isDarkMode());
        //qDebug() << "Theme changed. Dark mode?" << isDarkMode();
    }
    return QWidget::event(e);
}

void AboutDialog::setLogoDarkMode(bool isDarkMode) {
    if(isDarkMode) {
        ui->logo->setStyleSheet("image: url(:/Resources/Images/AcademyScopeDarkMode.png);");
    }
    else {
        ui->logo->setStyleSheet("image: url(:/Resources/Images/AcademyScope.png);");
    }
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

