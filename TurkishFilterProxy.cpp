/*
TurkishFilterProxy class definitions of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "TurkishFilterProxy.hpp"

TurkishFilterProxy::TurkishFilterProxy(QObject *parent)
        : QSortFilterProxyModel(parent),
        turkish(QLocale::Turkish, QLocale::Turkey),
        collator(QLocale(QLocale::Turkish, QLocale::Turkey))
    {
        collator.setCaseSensitivity(Qt::CaseInsensitive);
        setDynamicSortFilter(true);
    }

void TurkishFilterProxy::setNeedle(const QString &s) {
    needle = turkish.toLower(s);
    invalidateFilter();
}

bool TurkishFilterProxy::filterAcceptsRow(int row, const QModelIndex &parent) const {
        const QString hay = turkish.toLower(sourceModel()->index(row, 0, parent).data().toString());
        return needle.isEmpty() || hay.contains(needle);
    }

// Türkçe sıralama
bool TurkishFilterProxy::lessThan(const QModelIndex &l, const QModelIndex &r) const {
        const QString a = l.data().toString();
        const QString b = r.data().toString();
        return collator.compare(a, b) < 0;
}
