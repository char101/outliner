#include "databaseutil.h"
#include "sqlquery.h"

#include <QDebug>
#include <QFile>

DatabaseUtil::DatabaseUtil(const QString& dbPath): dbPath(dbPath), failed(false)
{
}

int DatabaseUtil::version()
{
    SqlQuery sql;
    sql.prepare("SELECT value FROM version");
    sql.exec();
    sql.next();
    return sql.value(0).toInt();
}

void DatabaseUtil::setVersion(int value)
{
    if (failed)
        return;

    SqlQuery sql;
    sql.prepare("UPDATE version SET value = :value");
    sql.bindValue(":value", value);
    if (!sql.exec())
        failed = true;
    else
        qDebug() << "Database version set to" << value;
}

void DatabaseUtil::runSql(const QString& sql)
{
    if (failed)
        return;

    SqlQuery query;
    query.prepare(sql);
    if (!query.exec())
        failed = true;
}

bool DatabaseUtil::tableExists(const QString& name)
{
    QSqlQuery sql;
    sql.prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = :name");
    sql.bindValue(":name", name);
    sql.exec();
    return sql.next();
}

void DatabaseUtil::initialize()
{
    if (!tableExists("version")) {
        runSql("CREATE TABLE version (value INTEGER NOT NULL)");
        runSql("INSERT INTO version VALUES (1)");
    }
    migrate();
}

void DatabaseUtil::backup()
{
    if (failed)
        return;

    QString newPath = QString("%0.%1").arg(dbPath).arg(version());
    if (QFile(newPath).exists())
        return;

    QFile dbFile(dbPath);
    if (!dbFile.copy(newPath)) {
        failed = true;
        qDebug() << "Failed backing up database file to" << newPath;
    }
}

void DatabaseUtil::migrate()
{
    int version = DatabaseUtil::version();
    qDebug() << "Database version" << version;

    QSqlDatabase db;
    db.transaction();

    switch(version) {
        case 1:
            if (!tableExists("list")) {
                runSql("CREATE TABLE list ("
                    "  id INTEGER NOT NULL PRIMARY KEY,"
                    "  name TEXT NOT NULL,"
                    "  weight INTEGER NOT NULL)");
                runSql("INSERT INTO list (name, weight) VALUES ('List 1', 1)");
            }
            if (!tableExists("list_item")) {
                runSql("CREATE TABLE list_item ("
                    "  id INTEGER NOT NULL PRIMARY KEY,"
                    "  list_id INTEGER NOT NULL REFERENCES list (id),"
                    "  content TEXT NOT NULL,"
                    "  note TEXT NULL,"
                    "  checkable BOOLEAN DEFAULT 0 NOT NULL,"
                    "  checked BOOLEAN NULL,"
                    "  checked_at DATETIME NULL,"
                    "  expanded BOOLEAN DEFAULT 0 NOT NULL,"
                    "  weight INTEGER NOT NULL,"
                    "  parent_id INTEGER NULL REFERENCES list_item (id),"
                    "  created DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,"
                    "  modified DATETIME NULL,"
                    "  attr_date DATETIME NULL,"
                    "  attr_priority INTEGER NULL)");
                runSql("CREATE INDEX idx_list_item_list ON list_item (list_id)");
                runSql("CREATE INDEX idx_list_item_parent ON list_item (parent_id)");
                runSql("INSERT INTO list_item (list_id, content, weight) VALUES (1, 'Welcome', 0)");
            }
            setVersion(2);
        case 2:
            backup();
            runSql("ALTER TABLE list_item ADD highlight int");
            setVersion(3);
    }

    if (failed)
        db.rollback();
    else
        db.commit();
}
