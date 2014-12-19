#include "listwidget.h"
#include "listtree.h"
#include "listoutliner.h"
#include "breadcrumb.h"
#include "debug.h"

#include <QAction>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardItem>

ListWidget::ListWidget(QWidget* parent) : QWidget(parent)
{
    _outliner = new ListOutliner(this);

    _tabWidget = new QTabWidget(this);
    _tabWidget->setStyleSheet("QTabWidget::pane { padding-bottom: 0 }");

    _splitter = new QSplitter(this);
    _splitter->addWidget(_outliner);
    _splitter->addWidget(_tabWidget);

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_splitter);

    // update outline when changing list
    connect(this, &ListWidget::listSelected, _outliner, &ListOutliner::loadOutline);

    // click on outline
    connect(_outliner->tree(), &ListOutlinerTree::itemClickedAlt, [this](QTreeWidgetItem* item, int column) {
        if (column == 0)
            if (isZoomed())
                zoomTo(item->data(0, Qt::UserRole).toInt());
            else
                scrollTo(item->data(0, Qt::UserRole).toInt());
    });

    // double click on outline
    connect(_outliner->tree(), &ListOutlinerTree::itemDoubleClickedAlt, [this](QTreeWidgetItem* item, int column) {
        if (column == 0)
            zoomTo(item->data(0, Qt::UserRole).toInt());
    });
}

void ListWidget::loadLists()
{
    QSqlQuery sql("SELECT id, name FROM list ORDER BY weight ASC");
    int nlist = 0;
    while (sql.next()) {
        ++nlist;
        int listId = sql.value(0).toInt();

        ListTree* tree = new ListTree(listId, this);
        _trees.append(tree);

        Breadcrumb* breadcrumb = new Breadcrumb(this);
        breadcrumb->setVisible(false);

        connect(tree, &ListTree::zoomed, [this, breadcrumb, tree](ListItem* item) {
            // collect all items from this item to the root
            QList<ListItem*> items;
            for (ListItem* curr = item; curr && !curr->isRoot(); curr = curr->parent())
                items << curr;

            breadcrumb->clear();

            // root item
            QAction* action = breadcrumb->addAction("Top");
            connect(action, &QAction::triggered, [this, tree]() {
                tree->unzoomAll();
            });
            breadcrumb->addSeparator();

            // add button for each item to the breadcrumb
            for (int i = items.length() - 1; i > 0; --i) {
                QAction* action = breadcrumb->addAction(items[i]->label());
                if (i > 0)
                    breadcrumb->addSeparator();
                connect(action, &QAction::triggered, [this, tree, items, i]() {
                    tree->unzoomTo(tree->model()->indexFromItem(items[i]));
                });
            }

            // current item
            breadcrumb->addCurrent(items[0]->label());
        });
        connect(tree, &ListTree::unzoomed, [breadcrumb]() {
            breadcrumb->popAction();
        });

        ListModel* model = tree->model();
        connect(model, &ListModel::projectRemoved, _outliner, &ListOutliner::reloadOutline);
        connect(model, &ListModel::projectChanged, _outliner, &ListOutliner::reloadOutline);
        connect(model, &ListModel::projectAdded, _outliner, &ListOutliner::reloadOutline);
        connect(model, &ListModel::scheduleChanged, this, &ListWidget::scheduleChanged);
        connect(model, &ListModel::operationError, this, &ListWidget::operationError);

        auto widget = new QWidget;
        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(breadcrumb);
        layout->addWidget(tree);
        widget->setLayout(layout);

        _tabWidget->addTab(widget, sql.value(1).toString());

        emit listSelected(listId);
    }

    if (nlist == 1)
        _tabWidget->tabBar()->hide();
}

ListTree* ListWidget::currentTree() const
{
    int index = _tabWidget->currentIndex();

    // no selected tab
    if (index == -1)
        return nullptr;

    // invalid index
    if (index < 0 || index > (_trees.length() - 1))
        return nullptr;

    return _trees[index];
}

int ListWidget::currentListId() const
{
    ListTree* tree = static_cast<ListTree*>(currentTree());
    if (tree)
        return tree->listId();
    return 0;
}

QList<ListTree*> ListWidget::trees() const
{
    return _trees;
}

void ListWidget::scrollTo(int itemId)
{
    // invalid id
    if (itemId == 0)
        return;
    ListTree* tree = currentTree();
    if (!tree)
        return;
    tree->unzoomAll();
    tree->scrollTo(itemId);
    tree->setFocus(Qt::MouseFocusReason);
}

void ListWidget::zoomTo(int itemId)
{
    // invalid id
    if (itemId == 0)
        return;
    ListTree* tree = currentTree();
    if (!tree)
        return;
    tree->unzoomAll();
    tree->zoom(itemId);
    tree->setFocus(Qt::MouseFocusReason);
}

bool ListWidget::isZoomed() const
{
    ListTree* tree = currentTree();
    if (!tree)
        return false;
    return tree->isZoomed();
}

void ListWidget::toggleHideCompleted()
{
    ListTree* tree = currentTree();
    if (!tree)
        return;

    bool isHidden = tree->isHidingCompleted();
    if (isHidden)
        tree->showCompleted();
    else
        tree->hideCompleted();

    QAction* menuItem = dynamic_cast<QAction*>(sender());
    if (menuItem && menuItem->isCheckable())
        menuItem->setChecked(isHidden ? false : true);
}

void ListWidget::expandAll()
{
    ListTree* tree = currentTree();
    if (!tree)
        return;
    tree->expandAll();
}

void ListWidget::collapseAll()
{
    ListTree* tree = currentTree();
    if (!tree)
        return;
    tree->collapseAll();
}
