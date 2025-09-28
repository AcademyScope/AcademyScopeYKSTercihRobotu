#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStandardPaths>
#include <QDebug>

#include "SQLiteUtil.hpp"

QString SQLiteUtil::resolveDatabasePath() {
#ifdef QT_DEBUG
    // Dev-friendly: load DB from the same directory as this .cpp file
    return QFileInfo(__FILE__).absolutePath() + "/Databases/YKS.sqlite";
#else
    #if defined(Q_OS_MAC)
        // macOS: inside app bundle Resources/Databases
        const QString appDir = QCoreApplication::applicationDirPath(); // .../MyApp.app/Contents/MacOS
        return QDir::cleanPath(QDir(appDir).filePath("../Resources/Databases/YKS.sqlite"));

    #elif defined(Q_OS_WIN) || defined(Q_OS_LINUX)
        // Windows/Linux: next to the executable in a Databases folder
        const QString appDir = QCoreApplication::applicationDirPath();
        return QDir::cleanPath(QDir(appDir).filePath("Databases/YKS.sqlite"));

    #elif defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
        // Mobile platforms: bundle/APK is read-only -> copy seed DB to AppDataLocation once
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        const QString target = QDir(dataDir).filePath("YKS.sqlite");

        if (!QFile::exists(target)) {
            // Provide a seed DB via Qt resource (add it to your .qrc)
            const QString seedPath = ":/seed/YKS.sqlite";  // <- adjust to your .qrc path
            if (QFile::exists(seedPath)) {
                if (!QFile::copy(seedPath, target)) {
                    qWarning() << "Failed to copy seed DB to" << target;
                } else {
                    QFile::setPermissions(target, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
                }
            } else {
                qWarning() << "Seed DB not found in resources:" << seedPath;
            }
        }
        return target;

    #else
        // Fallback: executable dir
        return QDir(QCoreApplication::applicationDirPath()).filePath("Databases/YKS.sqlite");
    #endif
#endif
}

QString SQLiteUtil::trOrderExprFor(const QString& col) {
    // Only text columns to be processed
    if (!(col == "UniversiteAdi" || col == "FakulteYuksekokulAdi" ||
          col == "ProgramAdi"    || col == "PuanTuru")) {
        return col;
    }

    struct Map { const char* from; const char* to; };
    static const Map m[] = {
                            {"Ç","CZ"}, {"ç","cz"},
                            {"Ğ","GZ"}, {"ğ","gz"},
                            {"İ","IZ"}, {"i","iz"},
                            {"I","IY"}, {"ı","iy"},
                            {"Ö","OZ"}, {"ö","oz"},
                            {"Ş","SZ"}, {"ş","sz"},
                            {"Ü","UZ"}, {"ü","uz"},
                            };

    QString expr = col;
    for (const auto& kv : m) {
        expr = QStringLiteral("REPLACE(%1,'%2','%3')").arg(expr, kv.from, kv.to);
    }
    return expr;
}
