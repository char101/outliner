#pragma once

#include "listmodel.h"

#include <QStandardItem>

class ListItem : public QStandardItem
{
public:
    // Does not support setting the text in the constructor
    // since the constructor will call the base class setText
    ListItem();
    QStandardItem* clone() const;

    ListItem* parent() const;
    ListItem* child(int row) const;

    ListModel* model() const;
    virtual void setMarkdown(const QString& text);

    int id() const { return data().toInt(); }
    QString markdown() const;
    QString plainText() const;

    QString label() const; // breadcrumb label

    int weight() const;

    bool operator<(const QStandardItem& other) const;
    void saveWeights();

    bool isProject() const;
    void setIsProject(bool isProject);
};
