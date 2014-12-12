#include "databaseutil.h"
#include "sqlquery.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

DatabaseUtil::DatabaseUtil(const QString& dbPath): dbPath(dbPath), failed(false)
{
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
            setVersion(db, 2);
        case 2:
            runSql("ALTER TABLE list_item ADD highlight INT");
            setVersion(db, 3);
        case 3:
            addColumn("list_item", "checkstate INTEGER DEFAULT 0 NOT NULL");
            runSql("UPDATE list_item SET checkstate = 2 WHERE checked = 1"); // handle partially checked state (cancelled)
            dropColumn("list_item", "checked");
            setVersion(db, 4);
        case 4:
            // remove checkable column, if checkstate is not null then it's checkable
            addColumn("list_item", "checkstate_new INTEGER");
            runSql("UPDATE list_item SET checkstate_new = CASE WHEN checkable THEN checkstate ELSE NULL END");
            dropColumn("list_item", QStringList{"checkstate", "checkable"});
            setVersion(db, 5);
        case 5:
            addColumn("list_item",  "is_project INTEGER DEFAULT 0 NOT NULL");
            setVersion(db, 6);
        case 6:
            addColumn("list_item", "due_date DATETIME");
            dropColumn("list_item", QStringList{"attr_date", "attr_priority"});
            setVersion(db, 7);
        case 7:
            addColumn("list_item", "is_highlighted INTEGER DEFAULT 0 NOT NULL");
            runSql("UPDATE list_item SET is_highlighted = CASE WHEN highlight IS NULL THEN 0 ELSE 1 END");

            addColumn("list_item", "is_checkable INTEGER DEFAULT 0 NOT NULL");
            addColumn("list_item", "is_completed INTEGER DEFAULT 0 NOT NULL");
            addColumn("list_item", "is_cancelled INTEGER DEFAULT 0 NOT NULL");
            runSql("UPDATE list_item SET "
                   "is_checkable = CASE WHEN checkstate IS NULL THEN 0 ELSE 1 END, "
                   "is_completed = CASE WHEN checkstate = 2 THEN 1 ELSE 0 END, "
                   "is_cancelled = CASE WHEN checkstate = 1 THEN 1 ELSE 0 END");

            addColumn("list_item", "is_expanded INTEGER DEFAULT 0 NOT NULL");
            runSql("UPDATE list_item SET is_expanded = expanded");

            addColumn("list_item", "parent_id_new INTEGER DEFAULT 0 NOT NULL");
            runSql("UPDATE list_item SET parent_id_new = CASE WHEN parent_id IS NULL THEN 0 ELSE parent_id END");

            dropColumn("list_item", QStringList{"highlight", "checkstate", "expanded", "parent_id"});

            setVersion(db, 8);
        case 8:
            addColumn("list_item", "due_date_new TEXT");
            dropColumn("list_item", "due_date");
            setVersion(db, 9);
        case 9:
            addColumn("list_item", "value INTEGER DEFAULT 0 NOT NULL");
            setVersion(db, 10);
        case 10:
            addColumn("list_item", "is_milestone INTEGER DEFAULT 0 NOT NULL");
            setVersion(db, 11);
        case 11:
            addColumn("list_item", "priority INTEGER DEFAULT 0 NOT NULL");
            setVersion(db, 12);
        case 12:
            addColumn("list_item", "completed_at DATETIME");

            addColumn("list_item", "created_at DATETIME");
            runSql("UPDATE list_item SET created_at = created");

            addColumn("list_item", "modified_at DATETIME");
            runSql("UPDATE list_item SET modified_at = modified");

            dropColumn("list_item", QStringList{"created", "modified"});

            setVersion(db, 13);
        case 13:
            dropColumn("list_item", "checked_at");
            setVersion(db, 14);
        case 14:
            dropColumn("list_item", "note");
            setVersion(db, 15);
    }

    if (!failed)
        db.commit(); // close the transaction opened by the last setVersion

    return !failed;
}

int DatabaseUtil::version()
{
    SqlQuery sql;
    sql.prepare("SELECT value FROM version");
    sql.exec();
    sql.next();
    return sql.value(0).toInt();
}

void DatabaseUtil::setVersion(QSqlDatabase& db, int value)
{
#ifndef QT_DEBUG
    if (!failed)
        backup(); // backup previous database version if there is no error in migration
#endif

    if (failed) {
        qDebug() << "Database change version" << value << "rolled back";
        db.rollback();
        return;
    }

    SqlQuery sql;
    sql.prepare("UPDATE version SET value = :value");
    sql.bindValue(":value", value);
    if (!sql.exec())
        failed = true;

    db.commit();

    // begin next transaction
    db.transaction();
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

void DatabaseUtil::addColumn(const QString& table, const QString& colspec)
{
    runSql(QString("ALTER TABLE %0 ADD %1").arg(table).arg(colspec));
}

void DatabaseUtil::dropColumn(const QString& table, const QString& column)
{
    dropColumn(table, QStringList{column});
}

void DatabaseUtil::dropColumn(const QString& table, const QStringList& columns)
{
    migrateTable(table, columns);
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
