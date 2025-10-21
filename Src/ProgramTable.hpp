#pragma once

#include <QObject>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <BackEnd.hpp>

class ProgramTable : public QTableWidget, public ProgramTableInterface
{
    Q_OBJECT
public:
    ProgramTable(QWidget *parent);
    void clearContents() override;
    void setRowCount(int rowCount) override;
    void setUpdatesEnabled(bool isEnabled) override;
    void setSortingEnabled(bool isEnabled) override;
    QTableWidgetItem * createItem(QString text, Qt::AlignmentFlag) override;
    void insertRow(int rowIndex) override;
    bool isColumnHidden(ProgramTableColumn column) override;
    bool isColumnHidden(int columnIndex) override;
    void setItem(int row, ProgramTableColumn column, QString text) override;
    void showColumn(ProgramTableColumn column) override;
    void hideColumn(ProgramTableColumn column) override;
    void setColumnWidth(ProgramTableColumn column, int width) override;
protected:
    QList<Qt::AlignmentFlag> columnAlignments;
    void setColumnAlignment(ProgramTableColumn column, Qt::AlignmentFlag alignment) override;
    void init() override;
};
