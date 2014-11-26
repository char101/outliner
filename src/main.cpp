#include "mainwindow.h"
#include "sqldatabase.h"
#include "databaseutil.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QFontDatabase>

void loadCustomFonts()
{
    QDir fontsDir("fonts");
    if (fontsDir.exists())
        for (QString fontFile : fontsDir.entryList(QDir::Files))
            if (fontFile.endsWith(".ttf") || fontFile.endsWith(".otf"))
                QFontDatabase::addApplicationFont(fontsDir.filePath(fontFile));
}

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

    QDir::setCurrent(QFileInfo(dbPath).absoluteDir().path());
    // loadCustomFonts();

    auto db = SqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    db.open();

    // without this, sqlite operation is very slow
    db.exec("PRAGMA synchronous = OFF");

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
