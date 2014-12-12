#include "scheduleitem.h"

#include <QDebug>

ScheduleItem::ScheduleItem() : _type(ScheduleItem::Root), _content("_root") {}

ScheduleItem::ScheduleItem(int value, int type, const QString& display) : _type(type)
{
    if (display.isNull())
        _content = QStringLiteral("<b>") + QString::number(value) + QStringLiteral("</b>");
    else
        _content = QStringLiteral("<b>") + display + QStringLiteral("</b>");
}

ScheduleItem::ScheduleItem(const QDate& date) : _type(ScheduleItem::Day), _dueDate(date)
{
    _content = QStringLiteral("<b>") + date.toString("dd") + QStringLiteral("</b>");
}

ScheduleItem::ScheduleItem(int id, const QString& content, const QDate& dueDate, bool isCheckable, bool isCompleted)
    : _type(ScheduleItem::Task),
      _id(id),
      _content(content),
      _dueDate(dueDate),
      _isCheckable(isCheckable),
      _isCompleted(isCompleted)
{
}

ScheduleItem::~ScheduleItem() { qDeleteAll(_children); }

void ScheduleItem::appendChild(ScheduleItem* item)
{
    item->setParent(this, _children.length());
    _children.append(item);
}

ScheduleItem* ScheduleItem::child(int row) const
{
    if (row >= 0 && row < _children.length())
        return _children.at(row);
    return nullptr;
}

void ScheduleItem::clear()
{
    qDeleteAll(_children);
    _children.clear();
}
