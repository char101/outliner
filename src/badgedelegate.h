#pragma once

#include <QStyledItemDelegate>

class BadgeDelegate : public QStyledItemDelegate
{
public:
    BadgeDelegate(QWidget* parent);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
