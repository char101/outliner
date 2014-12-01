#pragma once

#include "htmldelegate.h"

#include <QTreeView>

class HtmlDelegateTree : public HtmlDelegate
{
public:
    HtmlDelegateTree(QTreeView* parent); // parent is required
    QTreeView* parent() const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
