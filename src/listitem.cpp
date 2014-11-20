#include "listitem.h"
#include "markdownrenderer.h"
#include "constants.h"

#include <QDebug>
#include <QTextDocument>

static MarkdownRenderer renderer;

ListItem::ListItem() : QStandardItem()
{
}

QStandardItem* ListItem::clone() const
{
    return new ListItem(*this);
}

ListItem* ListItem::parent() const
{
    return static_cast<ListItem*>(QStandardItem::parent());
}

void ListItem::setMarkdown(const QString& value)
{
    setText(renderer.convert(value.trimmed()));
    // QStandardItem considers DisplayRole == EditRole
    setData(value, App::OriginalTextRole);
}

QString ListItem::markdown() const
{
    return data(App::OriginalTextRole).toString();
}

QString ListItem::label() const
{
    QTextDocument doc;
    doc.setHtml(text());
    QString plainText = doc.toPlainText().replace('\n', " ").replace('\r', " ");
    if (plainText.length() > 80) {
        plainText.truncate(40);
        plainText = plainText.trimmed() + "...";
    }
    return plainText;
}
