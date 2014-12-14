#pragma once

#include <QString>
#include <QDate>
#include <QList>
#include <QDebug>

class ScheduleItem
{
public:
    ScheduleItem(); // root
    ScheduleItem(int value, int type, const QString& content = QString()); // year/months
    ScheduleItem(const QDate& dueDate); // day
    ScheduleItem(int id, const QString& content, const QDate& dueDate, bool isCheckable, bool isCompleted); // task
    ~ScheduleItem();

    int id() const { return _id; };
    bool isRoot() const { return _type == ScheduleItem::Root; };
    bool isToday() const { return _type == ScheduleItem::Day && _dueDate == QDate::currentDate(); };

    QString content() const { return _content; };
    void setContent(const QString& content) { _content = content; };
    QString html() const { return _content; }; // TODO

    QDate dueDate() const { return _dueDate; };

    ScheduleItem* parent() const { return _parent; };
    void setParent(ScheduleItem* parent, int row) { _parent = parent; _row = row; };

    int row() { return _row; };
    void setRow(int row) { _row = row; };

    bool isCheckable() const { return _type == ScheduleItem::Task && _isCheckable; };
    bool isCompleted() const { return _type == ScheduleItem::Task && _isCompleted; };

    void appendChild(ScheduleItem* item);
    int childCount() const { return _children.length(); }
    ScheduleItem* child(int row) const;

    void clear();

    enum Type { Year, Month, Day, Task, Root };
private:
    int _id{0};
    bool _isCheckable{false};
    bool _isCompleted{false};
    int _type{0};
    int _row{0};

    ScheduleItem* _parent{nullptr};

    QString _content;
    QDate _dueDate;
    QList<ScheduleItem*> _children;
};
