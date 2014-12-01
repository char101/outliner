#pragma once

#include "htmldelegatetree.h"

class ListTree;

class ListItemDelegate : public HtmlDelegateTree
{
public:
    ListItemDelegate(ListTree* parent);
    ListTree* parent() const;
    QColor textColor(const QModelIndex& index) const override;
};
