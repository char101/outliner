#include "listtree.h"

#include "listmodel.h"
#include "listitemeditdialog.h"
#include "constants.h"
#include "utils.h"
//
#include <QMessageBox>
#include <QDebug>
#include <QAction>
#include <QFont>
#include <QMenu>
#include <QHeaderView>

ListTree::ListTree(int listId, QWidget* parent) : QTreeView(parent), _listId(listId)
{
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);

    itemDelegate = new ListItemDelegate(this);
    setItemDelegateForColumn(0, itemDelegate);

    ListModel* model = new ListModel(listId, this);
    setModel(model);
    connect(this, &ListTree::expanded, [this](const QModelIndex& index) {
        this->model()->itemFromIndex(index)->setExpandedDb(true);
    });
    connect(this, &ListTree::collapsed, [this](const QModelIndex& index) {
        this->model()->itemFromIndex(index)->setExpandedDb(false);
    });

    QHeaderView* header = this->header();
    header->setStretchLastSection(false);
    // header->resizeSection(0, 250);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->resizeSection(1, 60);

    resizeTimer.setSingleShot(true);
    connect(&resizeTimer, &QTimer::timeout, this, &ListTree::resizeDone);

    restoreExpandedState(model->root());
}

ListModel* ListTree::model() const
{
    return static_cast<ListModel*>(QTreeView::model());
}

ListItem* ListTree::currentItem() const
{
    return model()->itemFromIndex(currentIndex());
}

void ListTree::keyPressEvent(QKeyEvent* event)
{
    if (currentIndex().isValid())
        if (_itemKeyPress(currentItem(), event->key(), event->modifiers()))
            return;
    QTreeView::keyPressEvent(event);
}

bool ListTree::_itemKeyPress(ListItem* item, int key, Qt::KeyboardModifiers modifiers)
{
    ListModel* model = this->model();
    switch (key) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            editOrFocus(currentIndex());
            return true;
        case Qt::Key_Insert:
            if (modifiers & Qt::ControlModifier)
                appendItem(App::AppendChild);
            else if (modifiers & Qt::AltModifier)
                appendItem(App::AppendBefore);
            else
                appendItem(App::AppendAfter);
            return true;
        case Qt::Key_Delete:
            remove(currentIndex());
            return true;
        case Qt::Key_C: // checkable
            item->setCheckableDb(!item->isCheckable());
            model->itemChanged(item);
            return true;
        case Qt::Key_Space: // toggle checkbox
            item->setCompletedDb(!item->isCompleted());
            model->itemChanged(item);
            return true;
        case Qt::Key_E:
        case Qt::Key_F2:
            edit(currentIndex());
            return true;
        case Qt::Key_P: // toggle project
            item->setProjectDb(!item->isProject());
            model->itemChanged(item);
            emit itemIsProject(item);
            return true;
        case Qt::Key_X: // cancel item
            item->setCancelledDb(!item->isCancelled());
            model->itemChanged(item);
            return true;
        case Qt::Key_H: // highlight
            item->setHighlightedDb(!item->isHighlighted());
            model->itemChanged(item);
            return true;
        case Qt::Key_Z: // zoom
            if (modifiers == Qt::ShiftModifier)
                unzoom();
            else
                zoom(currentIndex());
            return true;
        case Qt::Key_Backspace:
            unzoom();
            return true;
        case Qt::Key_Up: // move up
            if (modifiers == Qt::ControlModifier) {
                moveVertical(App::Up);
                return true;
            }
            break;
        case Qt::Key_Down: // move down
            if (modifiers == Qt::ControlModifier) {
                moveVertical(App::Down);
                return true;
            }
            break;
        case Qt::Key_Left: // move left
            if (modifiers == Qt::ControlModifier) {
                moveHorizontal(App::Left);
                return true;
            }
            break;
        case Qt::Key_Right: // move right
            if (modifiers == Qt::ControlModifier) {
                moveHorizontal(App::Right);
                return true;
            }
            break;
    }
    return false;
}

// void ListTree::mousePressEvent(QMouseEvent* e)
// {
//     QTreeView::mousePressEvent(e);
//     switch (e->button()) {
//         case Qt::LeftButton:
//             qDebug() << __FUNCTION__ << currentIndex();
//             break;
//     }
// }

void ListTree::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        editOrFocus(currentIndex());
        return;
    }
    QTreeView::mouseDoubleClickEvent(e);
}

void ListTree::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndexList selection = selectedIndexes();
    if (selection.isEmpty())
        return;

    QModelIndex index = selection.first();
    if (!index.isValid())
        return;

    ListModel* model = this->model();
    ListItem* item = model->itemFromIndex(index);
    if (item->childCount() <= 1)
        return;

    QMenu menu;
    QAction* sortByStatusAction = menu.addAction(Util::findIcon("sort"), "Sort by status");
    QAction* sortByStatusAndContentAction = menu.addAction(Util::findIcon("sort"), "Sort by status and content");

    QAction* action = menu.exec(event->globalPos());
    if (!action)
        return;

    if (action == sortByStatusAction) {
        model->sort(item, App::SortByStatus);
    } else if (action == sortByStatusAndContentAction) {
        model->sort(item, App::SortByStatusAndContent);
    }
}

void ListTree::moveVertical(int dir)
{
    int offset = dir == App::Up ? -1 : 1;

    QModelIndex curr = currentIndex();
    if (!curr.isValid())
        return;
    QModelIndex next = curr.sibling(curr.row() + offset, curr.column());
    if (!next.isValid())
        return;

    QModelIndex newIndex = model()->moveItemVertical(curr, dir);
    setCurrentIndex(newIndex);

    restoreExpandedState(newIndex);
    restoreExpandedState(curr);
}

void ListTree::moveHorizontal(int dir)
{
    QModelIndex curr = currentIndex();

    if (!curr.isValid())
        return;
    if (dir == App::Left && !curr.parent().isValid())
        return; // cannot move top level item left
    if (dir == App::Right && !curr.sibling(curr.row() - 1, 0).isValid())
        return; // cannot move the first child right

    ListModel* model = this->model();
    ListItem* item = model->itemFromIndex(curr);
    QModelIndex newIndex = model->moveItemHorizontal(curr, dir);

    if (dir == App::Left)
        restoreExpandedState(item);
    else
        restoreExpandedState(item->parent());

    setCurrentIndex(newIndex);
}

void ListTree::appendItem(App::AppendMode mode)
{
    ListItemEditDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString text = dialog.text().trimmed();
    if (text.isEmpty())
        return;

    ListModel* model = this->model();
    QModelIndex idx = currentIndex();
    if (mode == App::AppendChild) {
        auto childIndex = model->appendChild(idx, text);
        if (childIndex.isValid()) {
            // model->itemFromIndex(childIndex.parent())->setExpandedDb(true);
            // setExpanded(idx.parent(), true);
            setCurrentIndex(childIndex);
        }
    } else {
        QModelIndex newIndex = model->appendAfter(idx, text, mode);
        setCurrentIndex(newIndex);
    }
}

void ListTree::remove(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    ListItem* item = model()->itemFromIndex(index);
    if (!item)
        return;

    QMessageBox mbox;
    mbox.setText("<b>Do you want to remove this item?</b>");
    mbox.setInformativeText(item->label());
    mbox.setIcon(QMessageBox::Warning);
    mbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if (mbox.exec() == QMessageBox::Ok) {
        model()->removeItem(index);
    }
}

void ListTree::edit(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    ListItem* item = static_cast<ListItem*>(model()->itemFromIndex(index));
    if (!item)
        return;
    ListItemEditDialog dialog;
    dialog.setText(item->markdown());
    if (dialog.exec() == QDialog::Accepted) {
        item->setMarkdown(dialog.text());
        model()->itemChanged(item);
    }
}

void ListTree::editOrFocus(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    ListItem* item = model()->itemFromIndex(index);
    if (!item)
        return;
    if (item->childCount() == 0)
        edit(index);
    else
        zoom(index);
}

void ListTree::zoom(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    ListItem* item = model()->itemFromIndex(index);
    if (!item)
        return;
    if (item->childCount() == 0) // no zoom for leaf node
        return;
    setRootIndex(index);
    emit zoomed(static_cast<ListItem*>(item));
}

void ListTree::zoom(int itemId)
{
    // invalid id
    if (itemId == 0)
        return;

    QModelIndex index = model()->indexFromId(itemId);
    // index not found
    if (!index.isValid())
        return;

    zoom(index);
}

void ListTree::unzoom()
{
    QModelIndex root = rootIndex();
    if (!root.isValid()) // already on the top level
        return;
    QModelIndex parent = root.parent();
    setRootIndex(parent);
    emit unzoomed();
}

void ListTree::unzoomTo(const QModelIndex& index)
{
    qDebug() << __FUNCTION__ << index;
    QModelIndex root = rootIndex();
    if (root == index)
        return;
    while (root.isValid() && root != index) {
        root = root.parent();
        setRootIndex(root);
        emit unzoomed();
    }
}

void ListTree::unzoomAll()
{
    qDebug() << __FUNCTION__;
    QModelIndex root = rootIndex();
    while (root.isValid()) {
        root = root.parent();
        setRootIndex(root);
        emit unzoomed();
    }
}

void ListTree::resizeEvent(QResizeEvent* event)
{
    resizeTimer.start(125);
    QTreeView::resizeEvent(event);
}

void ListTree::resizeDone()
{
    resizeIndexes(model()->root());
}

void ListTree::resizeIndexes(ListItem* item)
{
    auto model = this->model();
    for (int i = 0, n = item->childCount(); i < n; ++i) {
        auto child = item->child(i);
        auto index = model->indexFromItem(child);
        emit itemDelegate->sizeHintChanged(index);
        resizeIndexes(child);
    }
}

void ListTree::restoreExpandedState(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    ListItem* item = model()->itemFromIndex(index);
    if (!item)
        return;
    restoreExpandedState(item);
}

void ListTree::restoreExpandedState(ListItem* item)
{
    if (!item)
        return;
    setExpanded(model()->indexFromItem(item), item->isExpanded());
    for (int i = 0, n = item->childCount(); i < n; ++i) {
        restoreExpandedState(item->child(i));
    }
}

void ListTree::scrollTo(int itemId)
{
    // invalid id
    if (itemId == 0)
        return;

    QModelIndex index = model()->indexFromId(itemId);
    // index not found
    if (!index.isValid())
        return;

    QTreeView::scrollTo(index, QAbstractItemView::PositionAtTop);
    QItemSelectionModel* selection = selectionModel();
    selection->clear();
    selection->select(index, QItemSelectionModel::Select);
}
