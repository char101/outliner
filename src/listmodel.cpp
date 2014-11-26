#include <QStandardItem>
#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include "listmodel.h"
#include "listitem.h"
#include "sqlquery.h"

ListModel::ListModel(int listId, QObject* parent) : QStandardItemModel(0, 1, parent), listId(listId)
{
    setItemPrototype(new ListItem());

    _loadItems(invisibleRootItem());
    // setHorizontalHeaderLabels(QStringList() << "Lists");

    connect(this, &ListModel::rowsInserted, this, &ListModel::onRowsInserted);
    connect(this, &ListModel::itemChanged, this, &ListModel::onItemChanged);
    // connect(this, &ListModel::rowsAboutToBeRemoved, this, &ListModel::onRowsAboutToBeRemoved);
}

void ListModel::_loadItems(QStandardItem* parent, int parentId)
{
    SqlQuery sql;
    sql.prepare(QString("SELECT id, content, checkstate, expanded, highlight, is_project FROM list_item WHERE list_id = :list AND parent_id %0 ORDER BY weight ASC").arg(
        parentId == 0 ? "IS NULL" : "= :parent"
    ));
    if (parentId)
        sql.bindValue(":parent", parentId);
    sql.bindValue(":list", listId);
    sql.exec();
    while (sql.next()) {
        int itemId = sql.value(0).toInt();
        QString itemContent = sql.value(1).toString();
        QVariant checkState = sql.value(2);
        bool expanded = sql.value(3).toBool();
        QRgb highlight = sql.value(4).toInt();
        bool isProject = sql.value(5).toInt();

        bool checkable = !checkState.isNull();

        QList<QStandardItem*> items;
        auto item = new ListItem();
        item->setData(itemId);
        item->setMarkdown(itemContent);
        if (checkable) {
            item->setCheckable(true);
            item->setData(checkState, Qt::CheckStateRole);
        }
        item->setData(expanded, App::ExpandedStateRole);
        if (highlight) {
            item->setData(QColor(highlight), Qt::BackgroundRole);
        }
        item->setData(isProject, App::ProjectRole);
        if (isProject)
            item->setData(App::ProjectBackgroundColor, Qt::BackgroundRole);
        items << item;
        parent->appendRow(items);

        _loadItems(items[0], itemId);
    }
}

QModelIndex ListModel::appendAfter(const QModelIndex& index, QString text, App::AppendMode mode)
{
    if (!index.isValid())
        return QModelIndex();

    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return QModelIndex();

    QList<QStandardItem*> newItems;
    ListItem* newItem = new ListItem();
    newItem->setMarkdown(text);
    if (item->isCheckable())
        newItem->setCheckable(true);
    newItems << newItem;

    QStandardItem* parent = item->parent();
    if (!parent) {
        parent = invisibleRootItem();
    }
    if (mode == App::AppendAfter)
        parent->insertRow(index.row() + 1, newItems);
    else if (mode == App::AppendBefore)
        parent->insertRow(index.row(), newItems);

    return indexFromItem(newItem);
}

QModelIndex ListModel::appendChild(const QModelIndex& index, QString text)
{
    if (!index.isValid())
        return QModelIndex();
    QStandardItem* parent = itemFromIndex(index);
    if (!parent)
        return QModelIndex();

    QList<QStandardItem*> items;
    auto item = new ListItem();
    item->setMarkdown(text);
    items << item;

    parent->insertRow(parent->rowCount(), items);
    return indexFromItem(item);
}

void ListModel::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    QStandardItem* parentItem = parent.isValid() ? itemFromIndex(parent) : invisibleRootItem();

    for (int i = first; i <= last; ++i) {
        if (parentItem->child(i)->data().isValid())
            return; // moved row
    }

    QVariant parentId = parent.isValid() ? parentItem->data() : SQL_NULL;
    int rowCountBeforeInsert = parentItem->rowCount() - (last - first + 1);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    qDebug() << __FUNCTION__ << "first =" << first << "last =" << last << "rowCount() =" << parentItem->rowCount();

    // make room for weight
    if (first < rowCountBeforeInsert) {
        SqlQuery sql;
        sql.prepare(QString("UPDATE list_item SET weight = weight + :inc WHERE parent_id %0 AND weight >= :weight").arg(parent.isValid() ? "= :parent" : "IS NULL"));
        if (parent.isValid())
            sql.bindValue(":parent", parentId);
        sql.bindValue(":inc", last - first + 1);
        sql.bindValue(":weight", first);
        if (!sql.exec()) {
            db.rollback();
            return;
        }
    }

    for (int i = first; i <= last; ++i) {
        ListItem* childItem = static_cast<ListItem*>(parentItem->child(i));
        SqlQuery sql;
        sql.prepare("INSERT INTO list_item (list_id, parent_id, content, weight, checkstate) VALUES (:list, :parent, :content, :weight, :checkstate)");
        sql.bindValue(":list", listId);
        sql.bindValue(":parent", parentId);
        sql.bindValue(":weight", i);
        sql.bindValue(":content", childItem->markdown());
        sql.bindValue(":checkstate", childItem->isCheckable() ? childItem->data(Qt::CheckStateRole) : QVariant());
        if (!sql.exec()) {
            db.rollback();
            return;
        }
        childItem->setData(sql.lastInsertId());
    }

    db.commit();
}

void ListModel::onItemChanged(QStandardItem* item)
{
    ListItem* litem = static_cast<ListItem*>(item);
    QVariant data = litem->data();
    if (data.isValid()) {
        int id = data.toInt();
        QString content = litem->markdown();

        SqlQuery sql;
        sql.prepare("UPDATE list_item SET content = :content WHERE id = :id");
        sql.bindValue(":id", id);
        sql.bindValue(":content", content);
        sql.exec();
    } else {
        qDebug() << __FUNCTION__ << "Invalid data";
    }
}

void ListModel::removeItem(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    ListItem* item = static_cast<ListItem*>(itemFromIndex(index));
    if (!item)
        return;
    // disable removing the last child
    if (!item->parent() && invisibleRootItem()->rowCount() == 1)
        return;

    QMessageBox mbox;
    mbox.setText("<b>Do you want to remove this item?</b>");
    mbox.setInformativeText(item->label());
    mbox.setIcon(QMessageBox::Warning);
    mbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if (mbox.exec() == QMessageBox::Ok) {
        QSqlDatabase db;
        db.transaction();
        if (removeItem(item))
            db.commit();
        else
            db.rollback();
    }
}

bool ListModel::removeItem(QStandardItem* item)
{
    if (!item)
        return false;

    for (int i = item->rowCount(); i > 0; --i)
        removeItem(item->child(i - 1));

    int id = item->data().toInt();
    if (id == 0)
        return false;

    SqlQuery sql;
    sql.prepare("SELECT weight, parent_id FROM list_item WHERE id = :id");
    sql.bindValue(":id", id);
    if (!sql.exec())
        return false;
    if (!sql.next())
        return false;

    int weight = sql.value(0).toInt();
    int parentId = sql.value(1).toInt();

    sql.prepare(QString("UPDATE list_item SET weight = weight - 1 WHERE parent_id %0 AND weight > :weight").arg(
        parentId == 0 ? "IS NULL" : "= :parent"
    ));
    if (parentId)
        sql.bindValue(":parent", parentId);
    sql.bindValue(":weight", weight);
    if (!sql.exec())
        return false;

    sql.prepare("DELETE FROM list_item WHERE id = :id");
    sql.bindValue(":id", id);
    if (!sql.exec())
        return false;

    QStandardItem* parent = item->parent();
    if (parent == 0)
        parent = invisibleRootItem();
    parent->removeRow(item->row());
    return true;
}

void ListModel::setItemChecked(QStandardItem* item, bool checked)
{
    if (!item)
        return;

    int id = item->data().toInt();
    SqlQuery sql;
    sql.prepare(checked ?
                "UPDATE list_item SET checkstate = :checkstate, checked_at = CURRENT_TIMESTAMP WHERE id = :id" :
                "UPDATE list_item SET checkstate = :checkstate, checked_at = NULL WHERE id = :id");
    sql.bindValue("id", id);
    sql.bindValue("checkstate", checked ? Qt::Checked : Qt::Unchecked);
    sql.exec();
}

void ListModel::toggleItemCheckable(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return;
    int id = item->data().toInt();

    bool checkable = !item->isCheckable();

    SqlQuery sql;
    sql.prepare(checkable ?
                "UPDATE list_item SET checkstate = :checkstate WHERE id = :id":
                "UPDATE list_item SET checkstate = NULL, checked_at = NULL WHERE id = :id");
    sql.bindValue(":id", id);
    if (checkable)
        sql.bindValue("checkstate", Qt::Unchecked);
    if (!sql.exec())
        return;

    item->setCheckable(checkable);
    if (!checkable)
        item->setData(QVariant(), Qt::CheckStateRole);
}

void ListModel::toggleItemCheckState(const QModelIndex& index, Qt::CheckState checkedState)
{
    if (!index.isValid())
        return;
    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return;

    const int checkState = item->checkState();
    if (checkState != Qt::Unchecked && checkState != checkedState)
        return; // cannot cancel checked item and vice versa

    const int id = item->data().toInt();
    bool checked = checkState == checkedState;

    SqlQuery sql;
    if (!checked)
        sql.prepare("UPDATE list_item SET checkstate = :checkstate, checked_at = CURRENT_TIMESTAMP WHERE id = :id");
    else
        sql.prepare("UPDATE list_item SET checkstate = :checkstate, checked_at = NULL WHERE id = :id");
    sql.bindValue("id", id);
    sql.bindValue("checkstate", checkedState);
    if (!sql.exec())
        return;

    item->setCheckState(checked ? Qt::Unchecked : checkedState);
}

void ListModel::toggleItemChecked(const QModelIndex& index)
{
    toggleItemCheckState(index, Qt::Checked);
}

void ListModel::toggleItemCancelled(const QModelIndex& index)
{
    toggleItemCheckState(index, Qt::PartiallyChecked);
}

void ListModel::toggleHighlight(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return;

    int id = item->data().toInt();

    SqlQuery sql;
    sql.prepare("UPDATE list_item SET highlight = :highlight WHERE id = :id");
    sql.bindValue(":id", id);

    QColor highlightColor = Qt::yellow;
    QVariant currentValue = item->data(Qt::BackgroundRole);
    if (!currentValue.isValid())
        sql.bindValue(":highlight", highlightColor.rgb());
    else
        sql.bindValue(":highlight", SQL_NULL);

    if (sql.exec())
        if (!currentValue.isValid())
            item->setData(highlightColor, Qt::BackgroundRole);
        else
            item->setData(QVariant(), Qt::BackgroundRole);
}

void ListModel::toggleItemIsProject(const QModelIndex& index)
{
    if (!index.isValid() || index.column() != 0)
        return;
    ListItem* item = static_cast<ListItem*>(itemFromIndex(index));
    if (!item)
        return;

    // can only set a top level item or child of another project as a project
    ListItem* parent = item->parent();
    if (parent && !parent->isProject())
        return;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    if (_setItemIsProject(item, !item->isProject())) {
        db.commit();
        emit itemIsProject(item);
    } else
        db.rollback();
}

bool ListModel::_setItemIsProject(ListItem* item, bool isProject)
{
    int id = item->id();

    if (isProject) {
        SqlQuery sql;
        sql.prepare("UPDATE list_item SET is_project = 1 WHERE id = :id");
        sql.bindValue("id", id);
        if (!sql.exec())
            return false;
        item->setIsProject(true);
    } else {
        SqlQuery sql;
        sql.prepare("UPDATE list_item SET is_project = 0 WHERE id = :id");
        sql.bindValue("id", id);
        if (!sql.exec())
            return false;

        item->setIsProject(false);

        for (int i = 0, n = item->rowCount(); i < n; ++i) {
            ListItem* child = item->child(i);
            if (child->isProject())
                if (!_setItemIsProject(child, false))
                    return false;
        }
    }

    return true;
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole) {
        QStandardItem* item = itemFromIndex(index);
        if (item) {
            setItemChecked(item, value.toInt() == Qt::Checked);
        }
    }
    return QStandardItemModel::setData(index, value, role);
}

QModelIndex ListModel::moveItemVertical(const QModelIndex& index, int direction)
{
    QStandardItem* item = itemFromIndex(index);

    int row = item->row();
    int newRow = row + (direction == App::Up ? -1 : 1);

    if (direction == App::Up && row == 0)
        return index;

    QStandardItem* parent = item->parent();
    if (parent == 0)
        parent = invisibleRootItem();
    if (direction == App::Down && row == (parent->rowCount() - 1))
        return index;

    int id = item->data().toInt();
    if (!id)
        return QModelIndex();

    int otherId = parent->child(newRow)->data().toInt();

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    SqlQuery sql;
    sql.prepare("UPDATE list_item SET weight = :weight WHERE id = :id");

    sql.bindValue(":id", id);
    sql.bindValue(":weight", newRow);
    if (!sql.exec()) {
        db.rollback();
        return QModelIndex();
    }

    sql.bindValue(":id", otherId);
    sql.bindValue(":weight", row);
    if (!sql.exec()) {
        db.rollback();
        return QModelIndex();
    }

    db.commit();

    auto child = parent->takeChild(row);
    auto otherChild = parent->takeChild(newRow);
    parent->setChild(row, otherChild);
    parent->setChild(newRow, child);

    return indexFromItem(child);
}

QStandardItem* ListModel::moveItemHorizontal(QStandardItem* item, int dir)
{
    QStandardItem* parent = item->parent();
    int row = item->row();
    auto itemId = item->data();

    if (dir == App::Left) { // reparent as child of parent's parent
        if (!parent) // already top level item
            return nullptr;

        QStandardItem* newParent = parent->parent();
        QVariant newParentId = newParent ? newParent->data() : SQL_NULL;
        int newRow = parent->row() + 1;

        SqlQuery sql;
        sql.prepare("UPDATE list_item SET parent_id = :parent, weight = :weight WHERE id = :id");
        sql.bindValue(":parent", newParentId);
        sql.bindValue(":id", itemId);
        sql.bindValue(":weight", newRow);
        if (!sql.exec())
            return nullptr;

        auto temp = parent->takeRow(row);
        if (!newParent)
            newParent = invisibleRootItem();
        newParent->insertRow(newRow, temp);

        return newParent;
    } else { // move as child of previous sibling
        if (!parent)
            parent = invisibleRootItem();
        QStandardItem* previous = parent->child(row - 1);
        if (!previous)
            return nullptr;

        SqlQuery sql;
        sql.prepare("UPDATE list_item SET parent_id = :parent, weight = :weight WHERE id = :id");
        sql.bindValue(":id", itemId);
        sql.bindValue(":parent", previous->data());
        sql.bindValue(":weight", previous->rowCount());
        if (!sql.exec())
            return nullptr;

        auto temp = parent->takeRow(row);
        previous->insertRow(previous->rowCount(), temp);
        setExpandedState(previous, true);

        return previous;
    }
}

void ListModel::setExpandedState(QStandardItem* item, bool expanded)
{
    QVariant id = item->data();
    if (!id.isValid())
        return;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET expanded = :expanded WHERE id = :id");
    sql.bindValue(":expanded", expanded);
    sql.bindValue(":id", id);
    if (sql.exec())
        item->setData(expanded, App::ExpandedStateRole);

}

void ListModel::setExpandedState(const QModelIndex& index, bool expanded)
{
    setExpandedState(itemFromIndex(index), expanded);
}

void ListModel::sortChildren(QStandardItem* item)
{
    setSortMode(App::SortAll);
    item->sortChildren(0);
    saveItemWeights(item);
}

void ListModel::sortCompleted(QStandardItem* item)
{
    setSortMode(App::SortCompleted);
    item->sortChildren(0);
    saveItemWeights(item);
}

void ListModel::saveItemWeights(QStandardItem* parent)
{
    if (!parent)
        return;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    if (_saveItemWeights(parent))
        db.commit();
    else
        db.rollback();
}

bool ListModel::_saveItemWeights(QStandardItem* parent)
{
    int parentId = parent->data().toInt();

    qDebug() << __FUNCTION__ << parentId;

    SqlQuery sql;
    sql.prepare("SELECT id, weight FROM list_item WHERE parent_id = :parent");
    sql.bindValue("parent", parentId);
    if (!sql.exec())
        return false;

    QHash<int, int> dbWeights;
    while (sql.next()) {
        int id = sql.value(0).toInt();
        int weight = sql.value(1).toInt();
        dbWeights[id] = weight;
    }

    for (int row = 0, rowCount = parent->rowCount(); row < rowCount; ++row) {
        QStandardItem* child = parent->child(row);
        int id = child->data().toInt();
        if (dbWeights[id] != row) {
            sql.prepare("UPDATE list_item SET weight = :row WHERE id = :id");
            sql.bindValue("row", row);
            sql.bindValue("id", id);
            if (!sql.exec())
                return false;
        }
        if (child->rowCount() > 1)
            if (!_saveItemWeights(child))
                return false;
    }

    return true;
}

QModelIndex ListModel::indexFromId(int itemId) const
{
    QStandardItem* found = itemFromId(itemId);
    if (found)
        return indexFromItem(found);
    return QModelIndex();
}

QStandardItem* ListModel::itemFromId(int itemId, QStandardItem* parent) const
{
    if (!parent)
        parent = invisibleRootItem();
    for (int i = 0, n = parent->rowCount(); i < n; ++i) {
        QStandardItem* child = parent->child(i);
        if (child->data().toInt() == itemId)
            return child;
    }
    for (int i = 0, n = parent->rowCount(); i < n; ++i) {
        QStandardItem* found = itemFromId(itemId, parent->child(i));
        if (found)
            return found;
    }
    return nullptr;
}
