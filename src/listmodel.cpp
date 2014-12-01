#include "listmodel.h"

#include "listtree.h"
// #include "listitem.h"
#include "sqlquery.h"

#include <QDebug>

ListModel::ListModel(int listId, ListTree* parent) : QAbstractItemModel(parent), _listId(listId)
{
    _root = new ListItem(listId);
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
    sql.prepare("SELECT id, content, is_expanded, is_project, is_highlighted, is_checkable, is_completed, is_cancelled "
                "FROM list_item WHERE list_id = :list AND parent_id = :parent "
                "ORDER BY weight ASC");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parentId);
    if (!sql.exec())
        return;

    while (sql.next()) {
        int c = -1;
        int id = sql.value(++c).toInt();
        QString content = sql.value(++c).toString();
        bool isExpanded = sql.value(++c).toBool();
        bool isProject = sql.value(++c).toBool();
        bool isHighlighted = sql.value(++c).toBool();
        bool isCheckable = sql.value(++c).toBool();
        bool isCompleted = sql.value(++c).toBool();
        bool isCancelled = sql.value(++c).toBool();

        ListItem* item = new ListItem(_listId, id, content, isExpanded, isProject, isHighlighted, isCheckable, isCompleted, isCancelled);
        parent->appendChild(item);

        _loadItems(item);
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
    if (row >= parentItem->childCount())
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

    ListItem* parent = itemFromIndex(index)->parent();
    if (parent->isRoot()) // top level item
        return QModelIndex();

    return createIndex(parent->row(), 0, parent);
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ListItem* item = itemFromIndex(index);
    switch (index.column()) {
        case 0:
            switch (role) {
                case Qt::DisplayRole:
                    return item->html();
                case Qt::EditRole:
                    return item->markdown();
                case Qt::BackgroundRole:
                    if (item->isHighlighted())
                        return App::HighlightBackgroundColor;
                    else if (item->isProject())
                        return App::ProjectBackgroundColor;
                    else
                        return QVariant();
                case Qt::CheckStateRole:
                    if (item->isCheckable())
                        if (item->isCompleted())
                            return Qt::Checked;
                        else if (item->isCancelled())
                            return Qt::PartiallyChecked;
                        else
                            return Qt::Unchecked;
                    break;
            } break;
        case 1:
            switch (role) {
                case Qt::DisplayRole:
                    return "";
            } break;
    }
    return QVariant();
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    qDebug() << QString("%0:%1 (%2)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__) << role;
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
                        if (item->setCompletedDb(true)) {
                            itemChanged(item);
                            return true;
                        }
                        break;
                    case Qt::PartiallyChecked:
                        if (item->setCancelledDb(true)) {
                            itemChanged(item);
                            return true;
                        }
                        break;
                    case Qt::Unchecked:
                        if (item->setCompletedDb(false) && item->setCancelledDb(false)) {
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
            case 0: return "Outline";
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

ListItem* ListModel::itemFromIndex(const QModelIndex& index) const
{
    return index.isValid() ? static_cast<ListItem*>(index.internalPointer()) : _root;
};

QModelIndex ListModel::indexFromItem(ListItem* item) const
{
    return item->isRoot() ? QModelIndex() : createIndex(item->row(), 0, item);
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

    sql.prepare("INSERT INTO list_item (list_id, parent_id, weight, content) VALUES (:list, :parent, :weight, :content)");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parent->id());
    sql.bindValue(":weight", row);
    sql.bindValue(":content", content);
    if (!sql.exec())
        return QModelIndex();

    int id = sql.lastInsertId().toInt();

    beginInsertRows(indexFromItem(parent), row, row);
    ListItem* newItem = new ListItem(_listId, id, content);
    parent->appendChild(newItem);
    endInsertRows();

    return indexFromItem(newItem);
}

QModelIndex ListModel::appendChild(const QModelIndex& index, QString content)
{
    if (!index.isValid() || index.column() != 0)
        return QModelIndex();

    ListItem* parent = itemFromIndex(index);
    if (!parent)
        return QModelIndex(); // cannot append child to root, use append after

    int row = parent->childCount();

    SqlQuery sql;
    sql.prepare("INSERT INTO list_item (list_id, parent_id, weight, content) VALUES (:list, :parent, :weight, :content)");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parent->id());
    sql.bindValue(":weight", row);
    sql.bindValue(":content", content);
    if (!sql.exec())
        return QModelIndex();

    int id = sql.lastInsertId().toInt();

    beginInsertRows(indexFromItem(parent), row, row);
    ListItem* newItem = new ListItem(_listId, id, content);
    parent->appendChild(newItem);
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

QModelIndex ListModel::moveItemVertical(const QModelIndex& index, int direction)
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

    if (item->setRowDb(direction == App::Down ? row + 1 : row - 1) &&
        otherItem->setRowDb(row)) {
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
            item->setParentDbOnly(newParent, newRow)) {
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
            item->setParentDbOnly(newParent, newParent->childCount())) {
            db.commit();
            if (beginMoveRows(indexFromItem(parent), row, row, indexFromItem(newParent), newParent->childCount())) {
                newParent->appendChild(parent->takeChild(row));
                endMoveRows();
            }
            newParent->setExpandedDb(true);
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
}
