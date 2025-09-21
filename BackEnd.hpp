#pragma once
#include <QObject>


class Backend : public QObject {
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void onSwitchValueChanged(bool checked);
};
