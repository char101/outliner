#pragma once

#include <QTreeWidget>

class ListOutlinerTree : public QTreeWidget
{
    Q_OBJECT
public:
    ListOutlinerTree(QWidget* parent = 0);
signals:
    void itemClickedAlt(QTreeWidgetItem* item, int column);
    void itemDoubleClickedAlt(QTreeWidgetItem* item, int column);
private:
    bool preventItemClicked; // prevent item click just after double click
};
