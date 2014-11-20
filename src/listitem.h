#pragma once

#include <QStandardItem>

class ListItem : public QStandardItem
{
public:
    // Does not support setting the text in the constructor
    // since the constructor will call the base class setText
    ListItem();
    QStandardItem* clone() const;
    ListItem* parent() const;
    virtual void setMarkdown(const QString& text);
    QString markdown() const;
    QString label() const; // breadcrumb label
};
