#include "listitem.h"
#include "markdownrenderer.h"
#include "constants.h"

#include <QDebug>
#include <QTextDocument>

static MarkdownRenderer renderer;

ListItem::ListItem() : QStandardItem()
{
}

// for setItemPrototype
QStandardItem* ListItem::clone() const
{
    return new ListItem(*this);
}

ListItem* ListItem::parent() const
{
    return static_cast<ListItem*>(QStandardItem::parent());
}

ListItem* ListItem::child(int row) const
{
    return static_cast<ListItem*>(QStandardItem::child(row));
}

ListModel* ListItem::model() const
{
    return static_cast<ListModel*>(QStandardItem::model());
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

QString ListItem::plainText() const
{
    QTextDocument doc;
    doc.setHtml(text());
    return doc.toPlainText();
}

int ListItem::weight() const
{
    int w = 0;
    switch (checkState()) {
        case Qt::Checked:
            w += 10;
            break;
        case Qt::PartiallyChecked:
            w += 20;
            break;
    }
    return w;
}

bool ListItem::operator<(const QStandardItem& other) const
{
    const ListItem& ot = static_cast<const ListItem&>(other);
    int d = weight() - ot.weight();
    if (d == 0 && model()->sortMode() == App::SortAll)
        d = plainText().compare(ot.plainText());
    return d < 0;
}

bool ListItem::isProject() const
{
    return data(App::ProjectRole).toBool();
}

void ListItem::setIsProject(bool isProject)
{
    setData(isProject, App::ProjectRole);
    setData(isProject ? App::ProjectBackgroundColor : QVariant(), Qt::BackgroundRole);
}
