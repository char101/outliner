#pragma once

#include <QString>

class DatabaseUtil
{
public:
    DatabaseUtil(const QString& dbPath);
    void initialize();
    void migrate();
    void backup();
    int version();
    void setVersion(int value);
    void runSql(const QString& sql);
    bool tableExists(const QString& table);
private:
    QString dbPath;
    bool failed;
};
