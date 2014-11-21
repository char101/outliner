#include "mainwindow.h"
#include "databaseutil.h"

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    QApplication app(argc, argv);

    Q_INIT_RESOURCE(app);

    QCommandLineParser parser;
    parser.addPositionalArgument("db", "Database path");
    parser.process(app);

    const QStringList args = parser.positionalArguments();

    QString dbPath;
    if (args.length() > 0)
        dbPath = args[0];
    else
        dbPath = app.applicationDirPath() + "/" + "outline.sqlite";

    qDebug() << "Database path" << dbPath;

    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    db.open();

    // without this, sqlite operation is very slow
    db.exec("PRAGMA synchronous = OFF");

    DatabaseUtil dbUtil(dbPath);
    dbUtil.initialize();

    MainWindow win;
    win.setDatabasePath(dbPath);
    win.show();
    return app.exec();
}
