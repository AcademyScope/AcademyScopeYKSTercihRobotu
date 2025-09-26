/*
TurkishFilterProxy class declarations of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <QSortFilterProxyModel>
#include <QCollator>
#include <QLocale>
#include <QCompleter>
#include <QComboBox>
#include <QObject>

class TurkishFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit TurkishFilterProxy(QObject *parent=nullptr);

    void setNeedle(const QString &s);

protected:
    // contains eşleşmesi (ı/I, i/İ doğru çalışır)
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

    // Türkçe sıralama
    bool lessThan(const QModelIndex &l, const QModelIndex &r) const override;

private:
    QLocale turkish;
    QCollator collator;
    QString needle;
};
