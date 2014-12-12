#pragma once

#include "htmldelegatetree.h"

class ScheduleTree;

class ScheduleItemDelegate : public HtmlDelegateTree
{
public:
    ScheduleItemDelegate(ScheduleTree* parent);
    ScheduleTree* parent() const;
    QColor textColor(const QModelIndex& index) const override;
};
