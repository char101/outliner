#include "mainwindow.h"

#include "databaseutil.h"
#include "settings.h"
#include "debug.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QSqlQuery>
#include <QtPlugin>

void clearDb(const QString& dbPath)
{
    QFile dbFile(dbPath);
    if (dbFile.exists()) {
        qDebug() << "Removing" << dbPath;
        dbFile.remove();
    }
}

int main(int argc, char *argv[])
{
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    QApplication app(argc, argv);

    Q_INIT_RESOURCE(app);

    QCommandLineParser parser;
    parser.addPositionalArgument("db", "Database path");
    QCommandLineOption themeOpt("t", "Icons theme directory", "theme");
    parser.addOption(themeOpt);
    parser.process(app);

    const QStringList args = parser.positionalArguments();

    QString dbPath;
    if (args.length() > 0)
        dbPath = args[0];
    else {
#ifdef QT_DEBUG
        dbPath = "Q:\\Outliner\\test.sqlite";
#else
        dbPath = app.applicationDirPath() + "/" + "outline.sqlite";
#endif
    }

    if (parser.isSet(themeOpt))
        runtimeSettings["theme"] = parser.value(themeOpt);
    else {
        QDir iconsDir("icons");
        if (iconsDir.exists())
            runtimeSettings["theme"] = "icons";
    }

    QDir::setCurrent(QFileInfo(dbPath).absoluteDir().path());
    // Util::loadCustomFonts();

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    db.open();

    db.exec("PRAGMA journal_mode = WAL"); // write once to the wal log, merge on exit
    db.exec("PRAGMA synchronous = NORMAL"); // sync on checkpoint

    DatabaseUtil dbUtil(dbPath);
    if (dbUtil.initialize()) {
        MainWindow win;
        win.setDatabasePath(dbPath);
        win.show();
        return app.exec();
    } else {
        QMessageBox mbox;
        mbox.setText("Database initialization failed.");
        mbox.setIcon(QMessageBox::Critical);
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.exec();
        return 1;
    }
}