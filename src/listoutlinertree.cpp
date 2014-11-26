#include "listoutlinertree.h"

#include <QTimer>
#include <QApplication>

ListOutlinerTree::ListOutlinerTree(QWidget* parent) : QTreeWidget(parent), preventItemClicked(false)
{
    setExpandsOnDoubleClick(false);

    connect(this, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        if (!preventItemClicked)
            emit itemClickedAlt(item, column);
    });

    connect(this, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem* item, int column) {
        preventItemClicked = true;
        emit itemDoubleClickedAlt(item, column);
        QTimer::singleShot(QApplication::doubleClickInterval(), [this]() {
            preventItemClicked = false;
        });
    });
}
