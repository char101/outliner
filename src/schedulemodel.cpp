#include "schedulemodel.h"

#include "sqlquery.h"

#include <QColor>
#include <QDebug>

ScheduleModel::ScheduleModel(QObject* parent) : QAbstractItemModel(parent)
{
    _root = new ScheduleItem();
    _loadItems();
}

ScheduleModel::~ScheduleModel()
{
    delete _root;
}

void ScheduleModel::_loadItems()
{
    SqlQuery sql;
    sql.prepare("SELECT id, content, due_date, is_checkable, is_completed FROM list_item WHERE due_date IS NOT NULL AND is_cancelled = 0 ORDER BY due_date ASC");
    if (!sql.exec())
        return;

    QList<ScheduleItem*> items;
    QDate minDate;
    QDate maxDate;
    while (sql.next()) {
        int c = -1;

        int id = sql.value(++c).toInt();
        QString content = sql.value(++c).toString();
        QDate dueDate = sql.value(++c).toDate();
        bool isCheckable = sql.value(++c).toBool();
        bool isCompleted = sql.value(++c).toBool();

        ScheduleItem* item = new ScheduleItem(id, content, dueDate, isCheckable, isCompleted);
        items << item;

        if (minDate.isNull() || dueDate < minDate)
            minDate = dueDate;
        if (maxDate.isNull() || dueDate > maxDate)
            maxDate = dueDate;
    }

    if (items.length() == 0)
        return;

    QHash<int, ScheduleItem*> years;
    QHash<int, ScheduleItem*> months;
    QHash<QDate, ScheduleItem*> days;
    QDate currDate = minDate;
    while (currDate <= maxDate) {
        int y = currDate.year();
        if (!years.contains(y)) {
            years[y] = new ScheduleItem(y, ScheduleItem::Year);
            _root->appendChild(years[y]);
        }

        int m = currDate.month();
        int mm = y * 100 + m;
        if (!months.contains(mm)) {
            months[mm] = new ScheduleItem(m, ScheduleItem::Month, currDate.toString("MMMM"));
            years[y]->appendChild(months[mm]);
        }

        if (!days.contains(currDate)) {
            days[currDate] = new ScheduleItem(currDate);
            months[mm]->appendChild(days[currDate]);
        }

        currDate = currDate.addDays(1);
    }

    for (auto item : items) {
        days[item->dueDate()]->appendChild(item);
    }
}

int ScheduleModel::rowCount(const QModelIndex& parent) const
{
    return parent.column() > 0 ? 0 : itemFromIndex(parent)->childCount();
}

QModelIndex ScheduleModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0)
        return QModelIndex();

    ScheduleItem* parentItem = itemFromIndex(parent);
    if (!parentItem || row >= parentItem->childCount())
        return QModelIndex();

    ScheduleItem* childItem = parentItem->child(row);
    if (!childItem)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex ScheduleModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) // root
        return QModelIndex();

    ScheduleItem* parent = itemFromIndex(index)->parent();
    if (!parent) // top level item
        return QModelIndex();

    return indexFromItem(parent);
}

QVariant ScheduleModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {
        ScheduleItem* item = itemFromIndex(index);
        if (item)
            switch (role) {
                case Qt::BackgroundRole:
                    if (item->isToday())
                        return QColor(Qt::yellow);
                    break;
            }
            switch (index.column()) {
                case 0:
                    return _dataContent(item, role);
            }
    }
    return QVariant();
}

QVariant ScheduleModel::_dataContent(ScheduleItem* item, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return item->html();
        case Qt::CheckStateRole:
            if (item->isCheckable())
                return item->isCompleted() ? Qt::Checked : Qt::Unchecked;
            break;
#ifdef QT_DEBUG
        case Qt::ToolTipRole:
            return QString("id: %0 content: %1").arg(item->id()).arg(item->content());
#endif
    }
    return QVariant();
}

bool ScheduleModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid()) {
        ScheduleItem* item = itemFromIndex(index);
        if (item)
            switch (index.column()) {
                case 0:
                    return _setDataContent(item, value, role);
            }
    }
    return false;
}

bool ScheduleModel::_setDataContent(ScheduleItem* item, const QVariant& value, int role)
{
    return false;
}

QVariant ScheduleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                if (role == Qt::DisplayRole)
                    return "Task";
                break;
        }
    }
    return QVariant();
}

Qt::ItemFlags ScheduleModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        ScheduleItem* item = itemFromIndex(index);
        if (item && item->isCheckable())
            f |= Qt::ItemIsUserCheckable;
    }
    return f;
}

void ScheduleModel::clear()
{
    beginResetModel();
    _root->clear();
    endResetModel();
}

void ScheduleModel::reload()
{
    clear();
    _loadItems();
}
