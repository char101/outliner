#include "htmldelegatetree.h"

#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QDebug>
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

QColor HtmlDelegateTree::textColor(const QModelIndex& index) const
{
    QStandardItem* item = static_cast<QStandardItemModel*>(parent()->model())->itemFromIndex(index);
    if (item && item->checkState() == Qt::Checked)
        return QColor(Qt::gray);
    return QColor(Qt::black);
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
        // indentation/arrows
        int level = 1; // the top level items are already indented once
        QModelIndex curr = index.parent();
        while (curr.isValid()) {
            ++level;
            curr = curr.parent();
        }
        textWidth -= level * parent->indentation();

        // checkbox
        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            // http://code.woboq.org/qt5/qtbase/src/widgets/styles/qcommonstyle.cpp.html#937
            textWidth -= style->pixelMetric(QStyle::PM_IndicatorWidth, &option, parent) + style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, parent) + 1; // checkbox width + margin
        }

        // icon
        if (opt.features & QStyleOptionViewItem::HasDecoration) {
            // http://code.woboq.org/qt5/qtbase/src/widgets/styles/qcommonstyle.cpp.html#810
            textWidth -= opt.decorationSize.width() + style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, parent) + 1; // icon width + margin
        }
    }
    doc.setTextWidth(textWidth);
    return QSize(textWidth, doc.size().height());
}
