#include "listitemdelegate.h"

#include "listtree.h"

ListItemDelegate::ListItemDelegate(ListTree* parent) : HtmlDelegateTree(parent) {}

ListTree* ListItemDelegate::parent() const
{
    return static_cast<ListTree*>(QStyledItemDelegate::parent());
}

QColor ListItemDelegate::textColor(const QModelIndex& index) const
{
    ListItem* item = parent()->model()->itemFromIndex(index);

    if (item) {
        if (item->isCompleted())
            return QColor(Qt::gray);
        if (item->isCancelled())
            return QColor(182, 124, 124);
    }

    return QColor(Qt::black);
}
