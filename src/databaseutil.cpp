#include "databaseutil.h"
#include "sqlquery.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

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

void DatabaseUtil::setVersion(QSqlDatabase *db, int value)
{
    if (!failed)
        backup(); // backup previous database version if there is no error in migration

    if (failed) {
        qDebug() << "Database change version" << value << "rolled back";
        db->rollback();
        return;
    }

    SqlQuery sql;
    sql.prepare("UPDATE version SET value = :value");
    sql.bindValue(":value", value);
    if (!sql.exec())
        failed = true;

    qDebug() << "Database change version" << value << "commited";
    db->commit();

    // begin next transaction
    db->transaction();
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

void DatabaseUtil::backup()
{
    QFileInfo info(dbPath);

    QString newPath = QString("%0/%1.v%2.sqlite").arg(info.dir().path()).arg(info.baseName()).arg(version());
    if (QFile(newPath).exists())
        return;

    QFile dbFile(dbPath);
    if (!dbFile.copy(newPath)) {
        failed = true;
        qDebug() << "Failed backing up database file to" << newPath;
    }
}

bool DatabaseUtil::initialize()
{
    if (!tableExists("version")) {
        runSql("CREATE TABLE version (value INTEGER NOT NULL)");
        runSql("INSERT INTO version VALUES (1)");
        if (failed)
            return false;
    }
    return migrate();
}

/**
 * To rename column, create the new column (with _new prefix),
 * then call migrateTable dropping the old column
 */
bool DatabaseUtil::migrate()
{
    int version = DatabaseUtil::version();

    QSqlDatabase db = QSqlDatabase::database();
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
                    "  checked INT NULL,"
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
            setVersion(&db, 2);
        case 2:
            backup();
            runSql("ALTER TABLE list_item ADD highlight INT");
            setVersion(&db, 3);
        case 3:
            addColumn("list_item", "checkstate INTEGER DEFAULT 0 NOT NULL");
            runSql("UPDATE list_item SET checkstate = 2 WHERE checked = 1"); // handle partially checked state (cancelled)
            migrateTable("list_item", QStringList("checked"));
            setVersion(&db, 4);
        case 4:
            // remove checkable column, if checkstate is not null then it's checkable
            addColumn("list_item", "checkstate_new INTEGER");
            runSql("UPDATE list_item SET checkstate_new = CASE WHEN checkable THEN checkstate ELSE NULL END");
            migrateTable("list_item", QStringList() << "checkstate" << "checkable");
            setVersion(&db, 5);
        case 5:
            addColumn("list_item",  "is_project INTEGER DEFAULT 0 NOT NULL");
            setVersion(&db, 6);
    }

    if (!failed)
        db.commit(); // close the transaction opened by the last setVersion

    return !failed;
}

void DatabaseUtil::addColumn(const QString& table, const QString& colspec)
{
    runSql(QString("ALTER TABLE %0 ADD %1").arg(table).arg(colspec));
}

void DatabaseUtil::migrateTable(const QString& table, const QStringList& droppedColumns)
{
    if (failed)
        return;

    if (!migrateTableInner(table, droppedColumns))
        failed = true;
}

// remove droppped columns
// automatically migrate column_new to column
bool DatabaseUtil::migrateTableInner(const QString& table, const QStringList& droppedColumns)
{
    SqlQuery sql;

    sql.prepare(QString("PRAGMA table_info('%0')").arg(table)); // cannot bind
    if (!sql.exec())
        return false;

    QStringList oldColumns;
    QStringList newColumns;
    QStringList newColumnsSpec;
    while (sql.next()) {
        QString name = sql.value(1).toString();

        if (droppedColumns.contains(name))
            continue;

        QString newName = name;
        if (name.endsWith("_new"))
            newName = name.left(name.length() - 4);

        oldColumns << name;
        newColumns << newName;

        QString type = sql.value(2).toString();
        bool notnull = sql.value(3).toBool();
        QString defVal = sql.value(4).toString();
        bool isPk = sql.value(5).toBool();

        QStringList colSpec;
        colSpec << newName << type;
        if (notnull)
            colSpec << "NOT NULL";
        if (!defVal.isEmpty())
            colSpec << "DEFAULT" << defVal;
        if (isPk)
            colSpec << "PRIMARY KEY";
        newColumnsSpec << colSpec.join(' ');
    }

    QString newTable = table + "_temp";

    sql.prepare(QString("CREATE TABLE %0 (%1)").arg(newTable).arg(newColumnsSpec.join(',')));
    if (!sql.exec())
        return false;

    sql.prepare(QString("INSERT INTO %0 (%1) SELECT %2 FROM %3").
                arg(newTable).
                arg(newColumns.join(", ")).
                arg(oldColumns.join(", ")).
                arg(table)
    );
    if (!sql.exec())
        return false;

    sql.prepare(QString("DROP TABLE %0").arg(table));
    if (!sql.exec())
        return false;

    sql.prepare(QString("ALTER TABLE %0 RENAME TO %1").arg(newTable).arg(table));
    if (!sql.exec())
        return false;

    return true;
}
