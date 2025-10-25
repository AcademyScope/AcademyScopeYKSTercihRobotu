/*
XLSXExporter class definitions of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "XLSXExporter.hpp"
extern "C" {
#include <xlsxwriter.h>
}

bool XLSXExporter::exportModel(const QString &path, const QAbstractItemModel* model) {
    lxw_workbook* workbook = workbook_new(path.toUtf8().constData());
    if (!workbook) return false;

    lxw_worksheet* ws = workbook_add_worksheet(workbook, "Sonuçlar");
    lxw_format* bold = workbook_add_format(workbook);
    format_set_bold(bold);

    int rows = model->rowCount();
    int cols = model->columnCount();

    // Header row
    for (int c = 0; c < cols; ++c) {
        QString header = model->headerData(c, Qt::Horizontal).toString();
        worksheet_write_string(ws, 0, c, header.toUtf8().constData(), bold);
    }

    // Data
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QVariant val = model->data(model->index(r, c));
            if (val.canConvert<double>())
                worksheet_write_number(ws, r + 1, c, val.toDouble(), nullptr);
            else
                worksheet_write_string(ws, r + 1, c, val.toString().toUtf8().constData(), nullptr);
        }
    }

    workbook_close(workbook);
    return true;
}
