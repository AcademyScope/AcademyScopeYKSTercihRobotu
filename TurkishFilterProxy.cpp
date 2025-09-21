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
