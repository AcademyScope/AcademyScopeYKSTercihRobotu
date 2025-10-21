/*
ProgramTable class declarations of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
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
