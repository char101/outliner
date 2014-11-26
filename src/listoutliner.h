#pragma once

#include "listoutlinertree.h"

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

class ListOutliner : public QWidget
{
    Q_OBJECT
public:
    ListOutliner(QWidget* parent = 0);
    QSize sizeHint() const;
    ListOutlinerTree* tree;
public slots:
    void loadOutline(int listId);
private:
    QVBoxLayout* layout;
    void loadOutlineInner(int listId, int parentId = 0, QTreeWidgetItem* parent = 0);
};
