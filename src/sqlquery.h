#pragma once

#include <QSqlQuery>

// cannot extends QSqlQuery since the methods are not virtual
class SqlQuery
{
public:
    SqlQuery(const QString& query = QString());
    bool prepare(const QString& query);
    bool exec(const QString& query);
    bool exec();
    void bindValue(const QString& placeholder, const QVariant& val);
    bool next();
    bool first();
    QVariant value(int index) const;
    QVariant lastInsertId() const;

    bool fetch();
private:
    QSqlQuery _query;
};
