#include <QMessageBox>
#include <QDebug>

#include "listtree.h"
#include "listitemeditdialog.h"
#include "listitem.h"
#include "constants.h"

ListTree::ListTree(int listId, QWidget* parent) : QTreeView(parent), listId(listId)
{
    setHeaderHidden(true);
    setAlternatingRowColors(true);

    itemDelegate = new HtmlDelegateTree(this);
    setItemDelegate(itemDelegate);

    auto model = new ListModel(listId);
    setModel(model);
    connect(this, &ListTree::expanded, [this](const QModelIndex& index) {
        this->model()->setExpandedState(index, true);
    });
    connect(this, &ListTree::collapsed, [this](const QModelIndex& index) {
        this->model()->setExpandedState(index, false);
    });

    resizeTimer.setSingleShot(true);
    connect(&resizeTimer, &QTimer::timeout, this, &ListTree::resizeDone);

    restoreExpandedState();
}

ListModel* ListTree::model() const
{
    return static_cast<ListModel*>(QTreeView::model());
}

void ListTree::keyPressEvent(QKeyEvent* event)
{
    ListModel* model = this->model();
    auto modifiers = event->modifiers();
    switch (event->key()) {
        case Qt::Key_F2:
        case Qt::Key_Return:
        case Qt::Key_Enter:
            edit(currentIndex());
            return;
        case Qt::Key_Insert:
            appendItem(modifiers == Qt::ControlModifier);
            return;
        case Qt::Key_Delete:
            remove(currentIndex());
            return;
        case Qt::Key_C: // checkable
            model->toggleItemCheckable(currentIndex());
            return;
        case Qt::Key_H: // highlight
            model->toggleHighlight(currentIndex());
            return;
        case Qt::Key_Z: // zoom
            if (modifiers == Qt::ShiftModifier)
                unzoom();
            else
                zoom(currentIndex());
            return;
        case Qt::Key_Up: // move up
            if (modifiers == Qt::ControlModifier) {
                moveVertical(App::Up);
                return;
            }
            break;
        case Qt::Key_Down: // move down
            if (modifiers == Qt::ControlModifier) {
                moveVertical(App::Down);
                return;
            }
            break;
        case Qt::Key_Left: // move left
            if (modifiers == Qt::ControlModifier) {
                moveHorizontal(App::Left);
                return;
            }
            break;
        case Qt::Key_Right: // move right
            if (modifiers == Qt::ControlModifier) {
                moveHorizontal(App::Right);
                return;
            }
            break;
        case Qt::Key_Space: // toggle checkbox
            toggleCheckState(currentIndex());
            return;
    }
    QTreeView::keyPressEvent(event);
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
        QModelIndex index = currentIndex();
        if (!index.isValid())
            return;
        QStandardItem* item = model()->itemFromIndex(index);
        if (!item)
            return;
        if (item->rowCount() == 0)
            edit(index);
        else
            zoom(index);
        return;
    }
    QTreeView::mouseDoubleClickEvent(e);
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
    QStandardItem* item = model->itemFromIndex(curr);
    if (!item)
        return;

    QStandardItem* newParent = model->moveItemHorizontal(item, dir);

    if (dir == App::Left)
        restoreExpandedState(item);
    else
        restoreExpandedState(newParent);

    setCurrentIndex(model->indexFromItem(item));
}

void ListTree::appendItem(bool asChild)
{
    ListItemEditDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString text = dialog.text().trimmed();
    if (text.isEmpty())
        return;

    qDebug() << __FUNCTION__ << "text =" << text;

    ListModel* model = this->model();
    QModelIndex idx = currentIndex();
    if (asChild) {
        auto childIndex = model->appendChild(idx, text);
        setExpanded(idx, true);
        setCurrentIndex(childIndex);
    } else {
        QModelIndex newIndex = model->appendAfter(idx, text);
        setCurrentIndex(newIndex);
    }
}

void ListTree::remove(const QModelIndex& index)
{
    model()->removeItem(index);
}

void ListTree::edit(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    ListItem* item = static_cast<ListItem*>(model()->itemFromIndex(index));
    ListItemEditDialog dialog;
    dialog.setText(item->markdown());
    if (dialog.exec() == QDialog::Accepted) {
        item->setMarkdown(dialog.text());
    }
}

void ListTree::toggleCheckState(const QModelIndex& index)
{
    model()->toggleItemCheckState(index);
}

void ListTree::zoom(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    QStandardItem* item = model()->itemFromIndex(index);
    if (!item)
        return;
    if (item->rowCount() == 0) // no zoom for leaf node
        return;
    setRootIndex(index);
    emit zoomed(static_cast<ListItem*>(item));
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
    resizeIndexes(model()->invisibleRootItem());
}

void ListTree::resizeIndexes(QStandardItem* item)
{
    auto model = this->model();
    for (int i = 0, n = item->rowCount(); i < n; ++i) {
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
    QStandardItem* item = model()->itemFromIndex(index);
    if (!item)
        return;
    restoreExpandedState(item);
}

void ListTree::restoreExpandedState(QStandardItem* item)
{
    QStandardItemModel* model = this->model();
    if (!item)
        item = model->invisibleRootItem();
    else
        setExpanded(item->index(), item->data(App::ExpandedStateRole).toBool());
    for (int i = 0, n = item->rowCount(); i < n; ++i) {
        restoreExpandedState(item->child(i));
    }
}
