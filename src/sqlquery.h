#pragma once
#include <QSqlQuery>

class SqlQuery : public QSqlQuery
{
public:
    SqlQuery(const QString& query = QString());
    bool exec(const QString& query);
    bool exec();
    bool fetch();
};
