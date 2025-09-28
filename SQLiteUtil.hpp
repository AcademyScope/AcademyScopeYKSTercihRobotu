#pragma once
#include <QString>

class SQLiteUtil
{
public:
    static QString resolveDatabasePath();
    static QString trOrderExprFor(const QString& col);
};
