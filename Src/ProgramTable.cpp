#include "ProgramTable.hpp"

ProgramTable::ProgramTable(QWidget *parent) : QTableWidget(parent) {
    init();
}

void ProgramTable::clearContents() {
    QTableWidget::clearContents();
}

void ProgramTable::setRowCount(int rowCount)
{
    QTableWidget::setRowCount(rowCount);
}

void ProgramTable::setUpdatesEnabled(bool isEnabled)
{
    QTableWidget::setUpdatesEnabled(isEnabled);
}

void ProgramTable::setSortingEnabled(bool isEnabled)
{
    QTableWidget::setSortingEnabled(isEnabled);
}

QTableWidgetItem * ProgramTable::createItem(QString text, Qt::AlignmentFlag alignment)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    return item;
}

void ProgramTable::insertRow(int rowIndex)
{

}

bool ProgramTable::isColumnHidden(ProgramTableColumn column)
{
    return QTableWidget::isColumnHidden((int) column);
}

bool ProgramTable::isColumnHidden(int columnIndex)
{
    return QTableWidget::isColumnHidden(columnIndex);
}

void ProgramTable::setItem(int row, ProgramTableColumn column, QString text)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    //item->setTextAlignment(alignment);
    QTableWidget::setItem(row, (int) column, item);
}

void ProgramTable::showColumn(ProgramTableColumn column) {
    QTableWidget::showColumn((int) column);
}

void ProgramTable::hideColumn(ProgramTableColumn column) {
    QTableWidget::hideColumn((int) column);
}

void ProgramTable::setColumnWidth(ProgramTableColumn column, int width) {
    QTableWidget::setColumnWidth((int) column, width);
}

void ProgramTable::setColumnAlignment(ProgramTableColumn column, Qt::AlignmentFlag alignment)
{
    //QTableWidget::setColumnAlignment((int) column, width);
}

void ProgramTable::init()
{
}
