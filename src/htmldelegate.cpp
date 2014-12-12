#include "htmldelegate.h"
#include "debug.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
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

    const int top = opt.rect.top();
    const int height = opt.rect.height();
    int left = opt.rect.left();
    int width = opt.rect.width(); // the width of the item excluding the arrows but including the checkbox and icons
    if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
        const int padding = checkboxWidth(style, parent, opt);
        left += padding;
        width -= padding;
    }
    if (opt.features & QStyleOptionViewItem::HasDecoration) {
        const int padding = iconWidth(style, parent, opt);
        left += padding;
        width -= padding;
    }

    QTextDocument doc;
    // doc.setDefaultFont(QFont("Roboto", 9));
    doc.setDefaultStyleSheet("ul { margin-left: -36px; } ol { margin-left: -28px; }");

    doc.setTextWidth(width);
    doc.setHtml(opt.text);

    painter->save();
    opt.text = ""; // draw the item but with empty text
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); // the 4th argument is required for QStyle to determine the style used to paint the control

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

int HtmlDelegate::checkboxWidth(QStyle* style, QWidget* parent, const QStyleOptionViewItem& opt) const
{
    // http://code.woboq.org/qt5/qtbase/src/widgets/styles/qcommonstyle.cpp.html#937
    return style->pixelMetric(QStyle::PM_IndicatorWidth, &opt, parent) + style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, parent) + 1;
}

int HtmlDelegate::iconWidth(QStyle* style, QWidget* parent, const QStyleOptionViewItem& opt) const
{
    // http://code.woboq.org/qt5/qtbase/src/widgets/styles/qcommonstyle.cpp.html#810
    return opt.decorationSize.width() + style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, parent) + 5; // icon width + margin, 5 is for the spacing after the icon with the text
}
