#include "scheduleitemdelegate.h"

#include "scheduletree.h"
#include "schedulemodel.h"

ScheduleItemDelegate::ScheduleItemDelegate(ScheduleTree* parent) : HtmlDelegateTree(parent) {}

ScheduleTree* ScheduleItemDelegate::parent() const
{
    return static_cast<ScheduleTree*>(QStyledItemDelegate::parent());
}

QColor ScheduleItemDelegate::textColor(const QModelIndex& index) const
{
    ScheduleItem* item = static_cast<ScheduleModel*>(parent()->model())->itemFromIndex(index);

    if (item) {
        if (item->isCompleted())
            return QColor(Qt::gray);
    }

    return QColor(Qt::black);
}
