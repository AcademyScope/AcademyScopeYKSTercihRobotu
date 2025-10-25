/*
CSVExporter class definitions of AcademyScope
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
#include "CSVExporter.hpp"
#include <QFile>
#include <QTextStream>

bool CSVExporter::exportModel(const QString &path, const QAbstractItemModel* model) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    const int rows = model->rowCount();
    const int cols = model->columnCount();

    // Header
    for (int c = 0; c < cols; ++c) {
        out << "\"" << model->headerData(c, Qt::Horizontal).toString() << "\"";
        if (c < cols - 1) out << ",";
    }
    out << "\n";

    // Data
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QString val = model->data(model->index(r, c)).toString();
            val.replace("\"", "\"\"");
            out << "\"" << val << "\"";
            if (c < cols - 1) out << ",";
        }
        out << "\n";
    }
    return true;
}
