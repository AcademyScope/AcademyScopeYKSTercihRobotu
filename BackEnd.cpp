#include "BackEnd.hpp"
#include <QDebug>

void Backend::onSwitchValueChanged(bool checked) {
    qDebug() << "Switch durumu değişti:" << checked;
}
