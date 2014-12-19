#include "listmodel.h"

#include "listtree.h"
#include "sqlquery.h"
#include "utils.h"
#include "debug.h"

#include <QDateTime>

ListModel::ListModel(int listId, ListTree* parent) : QAbstractItemModel(parent), _listId(listId)
{
    _root = new ListItem(this, listId);
    _loadItems(_root);
}

ListModel::~ListModel()
{
    delete _root;
}

ListItem* ListModel::root() const { return _root; }

void ListModel::_loadItems(ListItem* parent)
{
    int parentId = parent->id();

    SqlQuery sql;
    sql.prepare("SELECT id, weight, content, is_expanded, is_project, is_milestone, is_highlighted, is_checkable, is_completed, is_cancelled, due_date, priority "
                "FROM list_item WHERE list_id = :list AND parent_id = :parent "
                "ORDER BY weight ASC");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parentId);
    if (!sql.exec())
        return;

    int currRow = 0;
    while (sql.next()) {
        int c = -1;
        int id = sql.value(++c).toInt();
        int row = sql.value(++c).toInt();
        QString content = sql.value(++c).toString();
        bool isExpanded = sql.value(++c).toBool();
        bool isProject = sql.value(++c).toBool();
        bool isMilestone = sql.value(++c).toBool();
        bool isHighlighted = sql.value(++c).toBool();
        bool isCheckable = sql.value(++c).toBool();
        bool isCompleted = sql.value(++c).toBool();
        bool isCancelled = sql.value(++c).toBool();
        QDate dueDate = sql.value(++c).toDate();
        int priority = sql.value(++c).toInt();

        // Fix gap between row in database
        if (row != currRow) {
            SqlQuery sql;
            sql.prepare("UPDATE list_item SET weight = :weight WHERE id = :id");
            sql.bindValue(":weight", currRow);
            sql.bindValue(":id", id);
            sql.exec();
        }

        ListItem* item = new ListItem(_listId, id, content, isExpanded, isProject, isMilestone, isHighlighted, isCheckable, isCompleted, isCancelled, dueDate, priority);
        parent->appendChild(item);

        _loadItems(item);

        ++currRow;
    }
}

int ListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    return itemFromIndex(parent)->childCount();
}

int ListModel::columnCount(const QModelIndex& parent) const
{
    return (parent.column()) > 0 ? 0 : 2;
}

QModelIndex ListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0)
        return QModelIndex();

    ListItem* parentItem = itemFromIndex(parent);
    if (!parentItem || row >= parentItem->childCount())
        return QModelIndex();

    ListItem* childItem = parentItem->child(row);
    if (!childItem)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex ListModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) // root
        return QModelIndex();

    ListItem* item = itemFromIndex(index);
    if (!item)
        return QModelIndex();

    ListItem* parent = item->parent();
    if (parent->isRoot()) // top level item
        return QModelIndex();

    return createIndex(parent->row(), 0, parent);
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {
        ListItem* item = itemFromIndex(index);
        if (item) {
            // all columns
            switch (role) {
                case Qt::BackgroundRole:
                    if (item->isHighlighted())
                        return App::HighlightBackgroundColor;
                    else if (item->isProject())
                        return App::ProjectBackgroundColor;
                    else if (item->isMilestone())
                        return App::MilestoneBackgroundColor;
                    else
                        return QVariant();
            }
            // specific column
            switch (index.column()) {
                case 0:
                    switch (role) {
                        case Qt::DisplayRole:
                            return item->html();
                        case Qt::EditRole:
                            return item->markdown();
                        case Qt::CheckStateRole:
                            if (item->isCheckable())
                                if (item->isCompleted())
                                    return Qt::Checked;
                                else if (item->isCancelled())
                                    return Qt::PartiallyChecked;
                                else
                                    return Qt::Unchecked;
                            break;
                        case Qt::DecorationRole:
                            if (item->isProject())
                                return Util::findIcon("project");
                            if (item->isMilestone())
                                return Util::findIcon("milestone");
                            if (item->isNote())
                                return Util::findIcon("note");
                            break;
#ifdef QT_DEBUG
                        case Qt::ToolTipRole:
                            return QString("id: %1 row: %2 parent: %3 milestone: %4 priority: %5").arg(item->id()).arg(item->row()).arg(item->parent()->id()).arg(item->isMilestone()).arg(item->priority());
#endif
                    } break;
                case 1:
                    switch (role) {
                        case Qt::DisplayRole:
                            return item->dueDate();
                    } break;
            }
        }
    }
    return QVariant();
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    ListItem* item = itemFromIndex(index);
    if (!item)
        return false;

    if (index.column() == 0) {
        switch (role) {
            case Qt::CheckStateRole:
                switch (value.toInt()) {
                    case Qt::Checked:
                        if (item->setCompleted(true)) {
                            itemChanged(item);
                            return true;
                        }
                        break;
                    case Qt::PartiallyChecked:
                        if (item->setCancelled(true)) {
                            itemChanged(item);
                            return true;
                        }
                        break;
                    case Qt::Unchecked:
                        if (item->setCompleted(false) && item->setCancelled(false)) {
                            itemChanged(item);
                            return true;
                        }
                        break;
                }
                break;
        }
    }
    return false;
}

QVariant ListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        switch (section) {
            case 0: return "Task";
            case 1: return "Due Date";
        }
    return QVariant();
}

Qt::ItemFlags ListModel::flags(const QModelIndex& index) const
{
    int flags = QAbstractItemModel::flags(index);
    if (index.isValid())
        flags |= itemFromIndex(index)->flags();
    return flags;
}

QModelIndex ListModel::indexFromId(int itemId) const
{
    ListItem* found = itemFromId(itemId);
    if (found)
        return indexFromItem(found);
    return QModelIndex();
}

ListItem* ListModel::itemFromId(int itemId, ListItem* parent) const
{
    if (!parent)
        parent = root();
    for (int i = 0, n = parent->childCount(); i < n; ++i) {
        ListItem* child = parent->child(i);
        if (child->id() == itemId)
            return child;
    }
    for (int i = 0, n = parent->childCount(); i < n; ++i) {
        ListItem* found = itemFromId(itemId, parent->child(i));
        if (found)
            return found;
    }
    return nullptr;
}

QModelIndex ListModel::appendAfter(const QModelIndex& index, QString content, App::AppendMode mode)
{
    if (!index.isValid() || index.column() != 0)
        return QModelIndex();

    ListItem* item = itemFromIndex(index);
    if (!item)
        return QModelIndex();

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QModelIndex newIndex = _appendAfter(item, content, mode);
    if (newIndex.isValid())
        db.commit();
    else
        db.rollback();
    return newIndex;
}

QModelIndex ListModel::_appendAfter(ListItem* item, const QString& content, App::AppendMode mode)
{
    SqlQuery sql;

    ListItem* parent = item->parent();
    if (!parent)
        return QModelIndex(); // cannot append after the root
    int row = mode == App::AppendAfter ? item->row() + 1 : item->row();

    sql.prepare("UPDATE list_item SET weight = weight + 1 WHERE list_id = :list AND parent_id = :parent AND weight >= :weight");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parent->id());
    sql.bindValue(":weight", row);
    if (!sql.exec())
        return QModelIndex();

    sql.prepare("INSERT INTO list_item (list_id, parent_id, weight, content, created_at) VALUES (:list, :parent, :weight, :content, CURRENT_TIMESTAMP)");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parent->id());
    sql.bindValue(":weight", row);
    sql.bindValue(":content", content);
    if (!sql.exec())
        return QModelIndex();

    int id = sql.lastInsertId().toInt();

    beginInsertRows(indexFromItem(parent), row, row);
    ListItem* newItem = new ListItem(_listId, id, content);
    parent->insertChild(row, newItem);
    if (isNewItemCheckable(item->parent(), row))
        newItem->setCheckable(true);
    endInsertRows();

    return indexFromItem(newItem);
}

QModelIndex ListModel::appendChild(const QModelIndex& parent, int row, QString content)
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    ListItem* parentItem = itemFromIndex(parent);
    if (!parentItem)
        return QModelIndex();

    SqlQuery sql;

    sql.prepare("UPDATE list_item SET weight = weight + 1 WHERE parent_id = :parent AND weight >= :row");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parentItem->id());
    sql.bindValue(":row", row);
    if (!sql.exec())
        return QModelIndex();

    sql.prepare("INSERT INTO list_item (list_id, parent_id, weight, content, created_at) VALUES (:list, :parent, :row, :content, CURRENT_TIMESTAMP)");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parentItem->id());
    sql.bindValue(":row", row);
    sql.bindValue(":content", content);
    if (!sql.exec())
        return QModelIndex();

    int id = sql.lastInsertId().toInt();

    beginInsertRows(parent, row, row);
    ListItem* newItem = new ListItem(_listId, id, content);
    if (isNewItemCheckable(parentItem, row))
        newItem->setCheckable(true);
    parentItem->insertChild(row, newItem);
    endInsertRows();

    return indexFromItem(newItem);
}

void ListModel::sort(ListItem* parent, App::SortMode mode)
{
    int count = parent->childCount();

    QVector<QPair<ListItem*, int>> sorting(count);
    for (int i = 0; i < count; ++i) {
        sorting[i].first = parent->child(i);
        sorting[i].second = i;
    }

    if (!parent->sort(mode))
        return;

    QModelIndexList oldIndexes;
    QModelIndexList newIndexes;
    for (QModelIndex idx : persistentIndexList()) {
        ListItem* item = itemFromIndex(idx);
        if (!item)
            continue;
        if (idx.row() != item->row()) {
            oldIndexes.append(idx);
            newIndexes.append(indexFromItem(item));
        }
    }

    qDebug() << __FUNCTION__ << oldIndexes;
    qDebug() << __FUNCTION__ << newIndexes;

    changePersistentIndexList(oldIndexes, newIndexes);
    emit layoutChanged();
}

QModelIndex ListModel::moveItemVertical(const QModelIndex& index, App::Direction direction)
{
    ListItem* item = itemFromIndex(index);
    if (!item)
        return index;

    int row = item->row();
    if (direction == App::Up && row == 0)
        return index;

    ListItem* parent = item->parent();
    if (direction == App::Down && row == (parent->childCount() - 1))
        return index;

    ListItem* otherItem = parent->child(direction == App::Up ? row - 1 : row + 1);
    int downRow = direction == App::Down ? row : row - 1; // the row that is being moved down

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    if (item->setRow(direction == App::Down ? row + 1 : row - 1) &&
        otherItem->setRow(row)) {
        db.commit();
        if (beginMoveRows(index.parent(), downRow, downRow, index.parent(), downRow + 2)) {
            parent->moveChild(downRow);
            endMoveRows();
            return indexFromItem(item);
        }
    } else
        db.rollback();
    return index;
}

QModelIndex ListModel::moveItemHorizontal(const QModelIndex& index, int direction)
{
    ListItem* item = itemFromIndex(index);
    if (!item)
        return index;

    ListItem* parent = item->parent();
    int row = item->row();

    if (direction == App::Left) { // reparent as child of parent's parent
        if (!parent || parent->isRoot()) // already top level item
            return index;

        ListItem* newParent = parent->parent();
        int newRow = parent->row() + 1;

        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();
        if (parent->takeChildDb(row) &&
            item->setParentDb(newParent, newRow)) {
            db.commit();
            if (beginMoveRows(indexFromItem(parent), row, row, indexFromItem(newParent), newRow)) {
                newParent->insertChild(newRow, parent->takeChild(row));
                endMoveRows();
            }
            return indexFromItem(item);
        } else {
            db.rollback();
            return index;
        }
    } else { // move as child of previous sibling
        ListItem* newParent = parent->child(row - 1);
        if (!newParent)
            return index;

        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();
        if (parent->takeChildDb(row) &&
            item->setParentDb(newParent, newParent->childCount())) {
            db.commit();
            if (beginMoveRows(indexFromItem(parent), row, row, indexFromItem(newParent), newParent->childCount())) {
                newParent->appendChild(parent->takeChild(row));
                endMoveRows();
            }
            newParent->setExpanded(true);
            return indexFromItem(item);
        } else {
            db.rollback();
            return index;
        }
    }
}

void ListModel::removeItem(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    ListItem* item = itemFromIndex(index);
    if (!item)
        return;

    bool isProject = item->isProject();

    ListItem* parent = item->parent();

    // disable removing the last child
    if (parent == root() && root()->childCount() == 1)
        return;

    int row = item->row();

    beginRemoveRows(indexFromItem(parent), row, row);
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    if (_removeItem(item)) {
        db.commit();
        if (isProject)
            emit projectRemoved();
    } else {
        db.rollback();
    }
    endRemoveRows();
}

bool ListModel::_removeItem(ListItem* item)
{
    for (int i = item->childCount(); i > 0; --i)
        _removeItem(item->child(i - 1));

    ListItem* parent = item->parent();
    int id = item->id();
    int row = item->row();

    SqlQuery sql;
    sql.prepare("UPDATE list_item SET weight = weight - 1 WHERE parent_id = :parent AND weight > :weight");
    sql.bindValue(":parent", parent->id());
    sql.bindValue(":weight", row);
    if (!sql.exec())
        return false;

    sql.prepare("DELETE FROM list_item WHERE id = :id");
    sql.bindValue(":id", id);
    if (!sql.exec())
        return false;

    parent->removeChild(row);
    return true;
}

void ListModel::itemChanged(ListItem* item, const QVector<int>& roles)
{
    QModelIndex itemIndex = indexFromItem(item);
    emit dataChanged(itemIndex, itemIndex, roles);
    if (item->isProject() || item->isMilestone())
        emit projectChanged(item);
}

bool ListModel::isNewItemCheckable(ListItem* parent, int row)
{
    if (row > 0) {
        ListItem* previous = parent->child(row - 1);
        if (previous)
            return previous->isCheckable();
    } else
        return parent->isProject() || parent->isMilestone();

    return false;
}
