#include "listtree.h"

#include "listmodel.h"
#include "listitemeditdialog.h"
#include "constants.h"
#include "utils.h"
#include "debug.h"

#include <QMessageBox>
#include <QAction>
#include <QFont>
#include <QMenu>
#include <QHeaderView>
#include <QApplication>
#include <QPainter>

ListTree::ListTree(int listId, QWidget* parent) : QTreeView(parent), _listId(listId)
{
    _itemDelegate = new ListItemDelegate(this);

    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setItemDelegateForColumn(0, _itemDelegate);

    ListModel* model = new ListModel(listId, this);
    setModel(model);
    connect(this, &ListTree::expanded, [this](const QModelIndex& index) {
        this->model()->itemFromIndex(index)->setExpanded(true);
    });
    connect(this, &ListTree::collapsed, [this](const QModelIndex& index) {
        this->model()->itemFromIndex(index)->setExpanded(false);
    });

    restoreExpandedState(model->root());
    hideCompleted();

    QHeaderView* header = this->header();
    header->setStretchLastSection(false);
    // header->resizeSection(0, 250);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->resizeSection(1, 60);

    _resizeTimer.setSingleShot(true);
    connect(&_resizeTimer, &QTimer::timeout, this, &ListTree::doItemsLayout);
}

ListModel* ListTree::model() const
{
    return static_cast<ListModel*>(QTreeView::model());
}

ListItem* ListTree::currentItem() const
{
    return model()->itemFromIndex(currentIndex());
}

ListItem* ListTree::rootItem() const
{
    return model()->itemFromIndex(rootIndex());
}

void ListTree::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    QModelIndex curr = currentIndex();

    if (curr.isValid()) {
        if (curr != rootIndex()) {
            ListItem* item = currentItem();
            if (item && _itemKeyPress(currentItem(), key, event->modifiers()))
                return;
        }
    } else {
        // keys that apply when no item is visible, i.e. when the list is empty or an item with no childrene is being set as root
            switch (key) {
                case Qt::Key_Insert: {
                    ListItem* currentRoot = rootItem();
                    if (currentRoot && currentRoot->childCount() == 0) {
                        _appendItem(App::AppendChild);
                        return;
                    }
                }; break;
                case Qt::Key_Backspace:
                    unzoom();
                    return;
            }
    }

    QTreeView::keyPressEvent(event);
}

bool ListTree::_itemKeyPress(ListItem* item, int key, Qt::KeyboardModifiers modifiers)
{
    ListModel* model = this->model();
    switch (key) {
        case Qt::Key_0: // priority
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
            if (item->setPriority(key - 0x30)) { // Key_1 = 0x31
                model->itemChanged(item);
                viewport()->repaint();
            }
            return true;
        case Qt::Key_A: // append
            if (modifiers & Qt::ShiftModifier)
                _appendItem(App::AppendBefore);
            else
                _appendItem(App::AppendAfter);
            return true;
        case Qt::Key_B: // append child
            _appendItem(App::AppendChild);
            return true;
        case Qt::Key_C: // checkable
            item->setCheckable(!item->isCheckable());
            model->itemChanged(item);
            return true;
        case Qt::Key_D: // delete
        case Qt::Key_Delete:
            remove(currentIndex());
            return true;
        case Qt::Key_Space: // toggle checkbox
            item->setCompleted(!item->isCompleted());
            model->itemChanged(item);
            return true;
        case Qt::Key_E:
        case Qt::Key_F2:
            edit(currentIndex());
            return true;
        case Qt::Key_M:
            item->setMilestone(!item->isMilestone());
            model->itemChanged(item);
            return true;
        case Qt::Key_P: // toggle project
            item->setProject(!item->isProject());
            model->itemChanged(item);
            return true;
        case Qt::Key_X: // cancel item
            item->setCancelled(!item->isCancelled());
            model->itemChanged(item);
            return true;
        case Qt::Key_H: // highlight
            item->setHighlighted(!item->isHighlighted());
            model->itemChanged(item);
            return true;
        case Qt::Key_S: // sort
            model->sort(item, App::SortByStatus);
            return true;
        case Qt::Key_Z: // zoom
            if (modifiers == Qt::ShiftModifier)
                unzoom();
            else
                zoom(currentIndex());
            return true;
        case Qt::Key_Insert:
            if (modifiers & Qt::ControlModifier)
                _appendItem(App::AppendChild);
            else if (modifiers & Qt::AltModifier)
                _appendItem(App::AppendBefore);
            else
                _appendItem(App::AppendAfter);
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            editOrFocus(currentIndex());
            return true;
        case Qt::Key_Backspace:
            unzoom();
            return true;
        case Qt::Key_Up: // move up
            if (modifiers == Qt::ControlModifier) {
                _moveVertical(App::Up);
                return true;
            }
            break;
        case Qt::Key_Down: // move down
            if (modifiers == Qt::ControlModifier) {
                _moveVertical(App::Down);
                return true;
            }
            break;
        case Qt::Key_Left: // move left
            if (modifiers == Qt::ControlModifier) {
                _moveHorizontal(App::Left);
                return true;
            }
            break;
        case Qt::Key_Right: // move right
            if (modifiers == Qt::ControlModifier) {
                _moveHorizontal(App::Right);
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

void ListTree::_moveVertical(int dir)
{
    int offset = dir == App::Up ? -1 : 1;

    QDEBUG << offset;

    QModelIndex curr = currentIndex();
    QDEBUG << curr;
    if (!curr.isValid())
        return;
    QModelIndex next = curr.sibling(curr.row() + offset, curr.column());
    QDEBUG << next;
    if (!next.isValid())
        return;

    QModelIndex newIndex = model()->moveItemVertical(curr, dir);
    QDEBUG << newIndex;
    setCurrentIndex(newIndex);

    restoreExpandedState(newIndex);
    restoreExpandedState(curr);
}

void ListTree::_moveHorizontal(int dir)
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

void ListTree::_appendItem(App::AppendMode mode)
{
    ListItemEditDialog dialog(QApplication::activeWindow(), "Add New Item");
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString text = dialog.text().trimmed();
    if (text.isEmpty())
        return;

    ListModel* model = this->model();

    QModelIndex idx = currentIndex();
    if (!idx.isValid()) {
        idx = rootIndex();
        mode = App::AppendChild;
    }

    QModelIndex newIndex;
    if (mode == App::AppendChild) {
        newIndex = model->appendChild(idx, _newItemRow(idx), text);
        if (newIndex.isValid()) {
            // model->itemFromIndex(childIndex.parent())->setExpanded(true);
            // setExpanded(idx.parent(), true);
            setCurrentIndex(newIndex);
        }
    } else {
        newIndex = model->appendAfter(idx, text, mode);
        if (newIndex.isValid())
            setCurrentIndex(newIndex);
    }

    QDate dueDate = dialog.dueDate();
    if (!dueDate.isNull() && newIndex.isValid())
        model->itemFromIndex(newIndex)->setDueDate(dueDate);
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
    if (mbox.exec() == QMessageBox::Ok)
        model()->removeItem(index);
}

void ListTree::edit(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    ListItem* item = model()->itemFromIndex(index);
    if (!item)
        return;

    ListItemEditDialog dialog(QApplication::activeWindow(), QStringLiteral("Edit Item %0").arg(item->id()));
    dialog.setText(item->markdown());
    dialog.setDueDate(item->dueDate());

    if (dialog.exec() == QDialog::Accepted) {
        item->setMarkdown(dialog.text());
        item->setDueDate(dialog.dueDate());
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

    if (item->isProject())
        zoom(index);
    else
        edit(index);
}

void ListTree::zoom(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    ListItem* item = model()->itemFromIndex(index);
    if (!item)
        return;

    if (item->childCount() == 0 && !item->isProject()) { // no zoom for leaf node
        scrollTo(item->id());
        return;
    }

    setRootIndex(index);
    setCurrentIndex(index);
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
    qDebug() << __FILE__ << __LINE__ << __FUNCTION__ ;
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

bool ListTree::isZoomed() const
{
    return rootIndex() != model()->indexFromItem(model()->root());
}

void ListTree::resizeEvent(QResizeEvent* event)
{
    _resizeTimer.start(125);
    QTreeView::resizeEvent(event); // let QTreeView resize columns
}

void ListTree::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QTreeView::drawBranches(painter, rect, index);

    if (!index.isValid() || index.column() != 0)
        return;

    ListItem* item = model()->itemFromIndex(index);
    if (item->isRoot())
        return;

    ListItem* root = model()->itemFromIndex(rootIndex());
    const int width = rect.width();
    const int nsteps = item->level() - root->level();
    const int step = width / nsteps;
    for (int i = nsteps - 1; i >= 0; --i) {
        _drawPriorityBar(item, painter, rect, step * i, i == nsteps - 1 ? 1 : 0);
        item = item->parent();
    }
}

void ListTree::_drawPriorityBar(ListItem* item, QPainter* painter, const QRect& rect, int x, int gap) const
{
    const int priority = item->priority();

    QColor color;
    switch (priority) {
        case 1: color = QColor(192, 57, 43); break;
        case 2: color = QColor(243, 156, 18); break;
        case 3: color = QColor(39, 174, 96); break;
        default: return;
    }

    color.setAlpha(192);
    painter->fillRect(x + 1, rect.y(), 1, rect.height() - gap, color);
    color.setAlpha(48);
    painter->fillRect(x, rect.y(), 1, rect.height() - gap, color);
    painter->fillRect(x + 2, rect.y(), 1, rect.height() - gap, color);
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

    QModelIndex leftIndex = model()->indexFromId(itemId);
    // index not found
    if (!leftIndex.isValid())
        return;

    QModelIndex rightIndex = leftIndex.sibling(leftIndex.row(), model()->columnCount(leftIndex));

    QTreeView::scrollTo(leftIndex, QAbstractItemView::PositionAtTop);
    QItemSelectionModel* selection = selectionModel();
    selection->clear();
    selection->select(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
}

void ListTree::hideCompleted()
{
    if (_isHidingCompleted)
        return;
    _hideCompletedRows();
    _isHidingCompleted = true;
}

void ListTree::_hideCompletedRows(ListItem* parent)
{
    ListModel* model = this->model();
    if (!parent)
        parent = model->root();
    QModelIndex parentIndex = model->indexFromItem(parent);
    for (int n = parent->childCount() - 1; n >= 0; --n) {
        ListItem* child = parent->child(n);
        if (child->isCompleted() || child->isCancelled())
            setRowHidden(child->row(), parentIndex, true);
        else if (child->childCount() > 0)
            _hideCompletedRows(child);
    }
}

void ListTree::showCompleted()
{
    if (!_isHidingCompleted)
        return;
    _showCompletedRows();
    _isHidingCompleted = false;
}

void ListTree::_showCompletedRows(ListItem* parent)
{
    ListModel* model = this->model();
    if (!parent)
        parent = model->root();
    QModelIndex parentIndex = model->indexFromItem(parent);
    for (int i = 0, n = parent->childCount(); i < n; ++i) {
        ListItem* child = parent->child(i);
        if (isRowHidden(child->row(), parentIndex))
            setRowHidden(child->row(), parentIndex, false);
        else if (child->childCount() > 0)
            _showCompletedRows(child);
    }
}

int ListTree::_newItemRow(const QModelIndex& parent)
{
    Q_ASSERT(parent.isValid());

    ListItem* item = model()->itemFromIndex(parent);
    if (!item)
        return 0;

    int pos = item->childCount();
    if (pos == 0)
        return 0;

    --pos;
    ListItem* child = nullptr;
    while (isRowHidden(pos, parent) || (child = item->child(pos), child->isCompleted() || child->isCancelled())) {
        --pos;
        if (pos < 0) // all child hidden
            return 0;
    }

    QDEBUG << pos + 1;

    return pos + 1;
}
