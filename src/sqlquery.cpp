#include <QDebug>
#include <QSqlError>
#include "sqlquery.h"

#define DEBUG_SQL 0

SqlQuery::SqlQuery(const QString& query) : QSqlQuery(query)
{
}

bool SqlQuery::exec(const QString& query)
{
    bool ret = QSqlQuery::exec(query);
#if DEBUG_SQL
    qDebug() << __FUNCTION__ << lastQuery();
#endif
    if (!ret) {
        qDebug() << __FUNCTION__ << "fail:" << lastQuery();
        qDebug() << __FUNCTION__ << "error:" << lastError().text();
    }
    return ret;
}

bool SqlQuery::exec()
{
    bool ret = QSqlQuery::exec();
#if DEBUG_SQL
    qDebug() << __FUNCTION__ << lastQuery();
#endif
    if (!ret) {
        qDebug() << __FUNCTION__ << "fail:" << lastQuery();
        qDebug() << __FUNCTION__ << "error:" << lastError().text();
    }
    return ret;
}

bool SqlQuery::fetch()
{
    if (!exec())
        return false;
    return first();
}
