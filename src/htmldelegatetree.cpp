#include "htmldelegatetree.h"
#include "debug.h"

#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QStandardItem>
#include <QStandardItemModel>

// static QHash<QString, QSize> sizeHintCache;

HtmlDelegateTree::HtmlDelegateTree(QTreeView* parent) : HtmlDelegate(parent)
{
}

QTreeView* HtmlDelegateTree::parent() const
{
    return static_cast<QTreeView*>(HtmlDelegate::parent());
}

QSize HtmlDelegateTree::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();

    QTreeView* parent = this->parent();

    QTextDocument doc;
    doc.setHtml(opt.text);

    const int column = index.column();
    int textWidth = parent->columnWidth(column);

    if (column == 0) {
        // indentation/arrows (only in column 0)
        int level = 1; // the top level items are already indented once
        QModelIndex root = parent->rootIndex();
        QModelIndex curr = index.parent();
        while (curr.isValid() && curr != root) {
            ++level;
            curr = curr.parent();
        }
        textWidth -= level * parent->indentation();
    }

    if (opt.features & QStyleOptionViewItem::HasCheckIndicator)
        textWidth -= checkboxWidth(style, parent, opt);
    if (opt.features & QStyleOptionViewItem::HasDecoration)
        textWidth -= iconWidth(style, parent, opt);

    doc.setTextWidth(textWidth);
    return QSize(textWidth, doc.size().height());
}
