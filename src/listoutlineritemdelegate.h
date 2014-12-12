#pragma once

#include "htmldelegatetree.h"

class ListOutlinerTree;

class ListOutlinerItemDelegate : public HtmlDelegateTree
{
public:
    ListOutlinerItemDelegate(ListOutlinerTree* parent);
};
