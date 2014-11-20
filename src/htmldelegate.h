#pragma once

#include <QStyledItemDelegate>
#include <QHash>
#include <QAbstractItemView>

class HtmlDelegate : public QStyledItemDelegate
{
public:
    HtmlDelegate(QAbstractItemView* parent); // parent is required
    virtual QAbstractItemView* parent() const;
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QColor textColor(const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
private:
    static int cbWidth;
};
