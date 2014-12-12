#include "listoutlinertree.h"

#include "debug.h"

#include <QTimer>
#include <QApplication>

ListOutlinerTree::ListOutlinerTree(QWidget* parent)
    : QTreeWidget(parent),
      _preventItemClicked(false)
{
    setColumnCount(1);
    setHeaderLabel("Outline");
    setExpandsOnDoubleClick(false);
    setAlternatingRowColors(true);

    _delegate = new ListOutlinerItemDelegate(this);
    setItemDelegateForColumn(0, _delegate);

    connect(this, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        QDEBUG;
        if (!_preventItemClicked) {
            QDEBUG << column;
            emit itemClickedAlt(item, column);
        }
    });

    connect(this, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem* item, int column) {
        _preventItemClicked = true;
        emit itemDoubleClickedAlt(item, column);
        QTimer::singleShot(QApplication::doubleClickInterval(), [this]() {
            _preventItemClicked = false;
        });
    });

    _resizeTimer.setSingleShot(true);
    connect(&_resizeTimer, &QTimer::timeout, this, &QTreeWidget::doItemsLayout);
}

void ListOutlinerTree::adjustDelegate() const
{
    emit _delegate->sizeHintChanged(indexFromItem(invisibleRootItem()->child(0)));
}

void ListOutlinerTree::resizeEvent(QResizeEvent* event)
{
    _resizeTimer.start(125);
    QTreeWidget::resizeEvent(event);
}
