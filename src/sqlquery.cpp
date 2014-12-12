#include "sqlquery.h"

#include "debug.h"

#include <QSqlError>
#include <QMessageBox>
#include <QApplication>

#define DEBUG_SQL 0

SqlQuery::SqlQuery(const QString& query)
{
    if (query.length()) {
        _query.prepare(query);
        _query.exec();
    }
}

bool SqlQuery::prepare(const QString& query)
{
    bool ret = _query.prepare(query);
    if (!ret) {
        QDEBUG << "fail:" << query;
        QDEBUG << "error:" << _query.lastError().text();
        QMessageBox::critical(QApplication::activeWindow(), __FUNCTION__, QString("%1\n%2").arg(_query.lastQuery(), _query.lastError().text()));
    }
    return ret;
}

bool SqlQuery::exec(const QString& query)
{
    bool ret = _query.exec(query);
#if DEBUG_SQL
    QDEBUG << _query.lastQuery();
#endif
    if (!ret) {
        QDEBUG << "fail:" << _query.lastQuery();
        QDEBUG << "error:" << _query.lastError().text();
        QMessageBox::critical(QApplication::activeWindow(), __FUNCTION__, QString("%1\n%2").arg(_query.lastQuery(), _query.lastError().text()));
    }
    return ret;
}

bool SqlQuery::exec()
{
    bool ret = _query.exec();
#if DEBUG_SQL
    QDEBUG << _query.lastQuery();
#endif
    if (!ret) {
        QDEBUG << "fail:" << _query.lastQuery();
        QDEBUG << "error:" << _query.lastError().text();
        QMessageBox::critical(QApplication::activeWindow(), __FUNCTION__, QString("%1\n%2").arg(_query.lastQuery(), _query.lastError().text()));
    }
    return ret;
}

bool SqlQuery::fetch()
{
    if (!exec())
        return false;
    return first();
}

void SqlQuery::bindValue(const QString& placeholder, const QVariant& val)
{
    if (placeholder[0] != ':') {
        QString temp = placeholder;
        temp.prepend(':');
        _query.bindValue(temp, val);
    } else
        _query.bindValue(placeholder, val);
}

bool SqlQuery::next()
{
    return _query.next();
}

bool SqlQuery::first()
{
    return _query.first();
}

QVariant SqlQuery::value(int index) const
{
    return _query.value(index);
}

QVariant SqlQuery::lastInsertId() const
{
    return _query.lastInsertId();
}
