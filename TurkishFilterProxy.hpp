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
