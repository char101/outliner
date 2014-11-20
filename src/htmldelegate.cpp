#include "htmldelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QTextDocument>

int HtmlDelegate::cbWidth = 0;

HtmlDelegate::HtmlDelegate(QAbstractItemView* parent) : QStyledItemDelegate(parent)
{
}

QAbstractItemView* HtmlDelegate::parent() const
{
    return static_cast<QAbstractItemView*>(QStyledItemDelegate::parent());
}

QColor HtmlDelegate::textColor(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QColor(Qt::black);
}

void HtmlDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();

    QAbstractItemView* parent = this->parent();

    painter->save();

    QTextDocument doc;
    doc.setDefaultStyleSheet("ul { margin-left: -36px; } ol { margin-left: -28px; }");

    doc.setHtml(opt.text);
    doc.setTextWidth(opt.rect.width());

    opt.text = "";

    // the 4th argument is required for QStyle to determine the style used to paint the control
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    const int top = opt.rect.top();
    const int height = opt.rect.height();
    int left = opt.rect.left();
    int width = opt.rect.width();
    if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
        const int padding = style->pixelMetric(QStyle::PM_IndicatorWidth, &option, parent) + style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, parent) + 1;
        left += padding;
        width += padding;
    }
    if (opt.features & QStyleOptionViewItem::HasDecoration) {
        const int padding = opt.decorationSize.width() + style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, parent) + 1; // icon width + margin
        left += padding;
        width += padding;
    }

    painter->translate(left, top);
    QRect clip(0, 0, width, height);
    painter->setClipRect(clip);

    QAbstractTextDocumentLayout::PaintContext context;
    context.clip = clip;
    const QColor textColor = this->textColor(index);
    context.palette.setColor(QPalette::AlternateBase, Qt::yellow);
    if (textColor != QColor(Qt::black)) {
        context.palette.setColor(QPalette::Text, textColor);
    }

    doc.documentLayout()->draw(painter, context);
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QTextDocument doc;
    doc.setHtml(opt.text);
    doc.setTextWidth(opt.rect.width());

    return QSize(doc.idealWidth(), doc.size().height());
}
