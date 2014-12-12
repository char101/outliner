#include "badgedelegate.h"

BadgeDelegate::BadgeDelegate(QWidget* parent)
    : QStyledItemDelegate(parent) {}

void BadgeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // painter->drawEllipse(x, y,
}
