/*
StringUtil class definitions of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "StringUtil.hpp"

QString StringUtil::toTurkishTitleCase(const QString &input)
{
    QStringList words = input.split(' ', Qt::SkipEmptyParts);
    for (QString &word : words) {
        if (!word.isEmpty()) {
            QString first = turkishLocale.toUpper(word.left(1));
            QString rest  = turkishLocale.toLower(word.mid(1));
            word = first + rest;
        }
    }
    return words.join(' ');
}

QString StringUtil::toTurkishUpperCase(const QString &input)
{
    return turkishLocale.toUpper(input);
}

QLocale StringUtil::turkishLocale = QLocale(QLocale::Turkish, QLocale::Turkey);
