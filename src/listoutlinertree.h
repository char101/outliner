#pragma once

#include "listoutlineritemdelegate.h"

#include <QTreeWidget>
#include <QTimer>

class ListOutlinerTree : public QTreeWidget
{
    Q_OBJECT
public:
    ListOutlinerTree(QWidget* parent = 0);
public slots:
    void adjustDelegate() const;
signals:
    void itemClickedAlt(QTreeWidgetItem* item, int column);
    void itemDoubleClickedAlt(QTreeWidgetItem* item, int column);
protected:
    void resizeEvent(QResizeEvent* event);
private:
    bool _preventItemClicked; // prevent item click just after double click
    ListOutlinerItemDelegate* _delegate;
    QTimer _resizeTimer;
};
