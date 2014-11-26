#pragma once

#include <QHash>
#include <QString>
#include <QSqlDatabase>

/**
 * Sqlite transaction handles DDL
 */

class DatabaseUtil
{
public:
    DatabaseUtil(const QString& dbPath);
    bool initialize();
    bool migrate();
    void backup();
    int version();
    void setVersion(QSqlDatabase *db, int value);
    void runSql(const QString& sql);
    bool tableExists(const QString& table);
    void addColumn(const QString& table, const QString& colspec);
    void migrateTable(const QString& table, const QStringList& droppedColumns);
private:
    QString dbPath;
    bool failed;
    bool migrateTableInner(const QString& table, const QStringList& droppedColumns);
};
