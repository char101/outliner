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

    loadItems(invisibleRootItem());
    // setHorizontalHeaderLabels(QStringList() << "Lists");

    connect(this, &ListModel::rowsInserted, this, &ListModel::onRowsInserted);
    connect(this, &ListModel::itemChanged, this, &ListModel::onItemChanged);
    // connect(this, &ListModel::rowsAboutToBeRemoved, this, &ListModel::onRowsAboutToBeRemoved);
}

void ListModel::loadItems(QStandardItem* parent, int parentId)
{
    SqlQuery sql;
    sql.prepare(QString("SELECT id, content, checkable, checked, expanded, highlight FROM list_item WHERE list_id = :list AND parent_id %0 ORDER BY weight ASC").arg(
        parentId == 0 ? "IS NULL" : "= :parent"
    ));
    if (parentId)
        sql.bindValue(":parent", parentId);
    sql.bindValue(":list", listId);
    sql.exec();
    while (sql.next()) {
        int itemId = sql.value(0).toInt();
        QString itemContent = sql.value(1).toString();
        bool checkable = sql.value(2).toBool();
        bool checked = sql.value(3).toBool();
        bool expanded = sql.value(4).toBool();
        QRgb highlight = sql.value(5).toInt();

        QList<QStandardItem*> items;
        auto item = new ListItem();
        item->setData(itemId);
        item->setMarkdown(itemContent);
        if (checkable) {
            item->setCheckable(true);
            if (checked)
                item->setData(checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        }
        item->setData(expanded, App::ExpandedStateRole);
        if (highlight) {
            item->setData(QColor(highlight), Qt::BackgroundRole);
        }
        items << item;
        parent->appendRow(items);

        loadItems(items[0], itemId);
    }
}

QModelIndex ListModel::appendAfter(const QModelIndex& index, QString text)
{
    QStandardItem* refItem = itemFromIndex(index);

    QList<QStandardItem*> items;
    auto item = new ListItem();
    item->setMarkdown(text);
    if (refItem->isCheckable())
        item->setCheckable(true);
    items << item;

    QStandardItem* parent = itemFromIndex(index)->parent();
    if (!parent) {
        parent = invisibleRootItem();
    }
    parent->insertRow(index.row() + 1, items);

    return indexFromItem(item);
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
        sql.prepare("INSERT INTO list_item (list_id, parent_id, content, weight, checkable) VALUES (:list, :parent, :content, :weight, :checkable)");
        sql.bindValue(":list", listId);
        sql.bindValue(":parent", parentId);
        sql.bindValue(":weight", i);
        qDebug() << __FUNCTION__ << "childItem->markdown() =" << childItem->markdown();
        sql.bindValue(":content", childItem->markdown());
        sql.bindValue(":checkable", childItem->isCheckable() ? 1 : 0);
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
    int id = item->data().toInt();
    SqlQuery sql;
    sql.prepare(checked ?
                "UPDATE list_item SET checked = 1, checked_at = CURRENT_TIMESTAMP WHERE id = :id AND checkable = 1" :
                "UPDATE list_item SET checked = 0, checked_at = NULL WHERE id = :id AND checkable = 1");
    sql.bindValue(":id", id);
    sql.exec();
}

void ListModel::setItemCheckable(const QModelIndex& index, bool checkable)
{
    if (!index.isValid())
        return;
    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return;
    int id = item->data().toInt();
    if (!id)
        return;
    SqlQuery sql;
    sql.prepare(checkable ?
                "UPDATE list_item SET checkable = 1, checked = 0 WHERE id = :id":
                "UPDATE list_item SET checkable = 0, checked = NULL, checked_at = NULL WHERE id = :id");
    sql.bindValue(":id", id);
    if (sql.exec()) {
        item->setCheckable(checkable);
        if (!checkable)
            item->setData(QVariant(), Qt::CheckStateRole);
    }
}

void ListModel::toggleItemCheckable(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return;
    setItemCheckable(index, !item->isCheckable());
}

void ListModel::toggleItemCheckState(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    QStandardItem* item = itemFromIndex(index);
    if (!item)
        return;
    const int id = item->data().toInt();
    const bool checked = item->checkState() == Qt::Checked;
    SqlQuery sql;
    sql.prepare(checked ? "UPDATE list_item SET checked = 0, checked_at = NULL WHERE id = :id" :
                          "UPDATE list_item SET checked = 1, checked_at = CURRENT_TIMESTAMP WHERE id = :id");
    sql.bindValue(":id", id);
    if (!sql.exec())
        return;
    item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
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

    QSqlDatabase db;
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
