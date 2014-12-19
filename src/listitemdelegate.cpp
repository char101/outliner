#include "listitemdelegate.h"

#include "listtree.h"
#include "constants.h"

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
            return App::CompletedColor;
        if (item->isCancelled())
            return App::CancelledColor;
        if (item->isNote())
            return App::NoteColor;
    }

    return QColor(Qt::black);
}
