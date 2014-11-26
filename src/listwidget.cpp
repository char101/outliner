#include "listwidget.h"
#include "listtree.h"
#include "breadcrumb.h"

#include <QAction>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardItem>

ListWidget::ListWidget(QWidget* parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    _tabWidget = new QTabWidget(this);
    _tabWidget->setStyleSheet("QTabWidget::pane { padding-bottom: 0 }");
    layout->addWidget(_tabWidget);
}

void ListWidget::loadLists()
{
    QSqlQuery sql("SELECT id, name FROM list ORDER BY weight ASC");
    int nlist = 0;
    while (sql.next()) {
        ++nlist;
        int listId = sql.value(0).toInt();

        auto tree = new ListTree(listId, this);
        _trees.append(tree);

        auto breadcrumb = new Breadcrumb(this);
        breadcrumb->setVisible(false);

        connect(tree, &ListTree::zoomed, [this, breadcrumb, tree](ListItem* item) {
            // collect all items from this item to the root
            QList<ListItem*> items;
            for (ListItem* curr = item; curr; curr = curr->parent())
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
                    tree->unzoomTo(items[i]->model()->indexFromItem(items[i]));
                });
            }

            // current item
            breadcrumb->addCurrent(items[0]->label());
        });
        connect(tree, &ListTree::unzoomed, [breadcrumb]() {
            breadcrumb->popAction();
        });

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
    qDebug() << __FUNCTION__ << itemId;
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
    qDebug() << __FUNCTION__ << itemId;
    // invalid id
    if (itemId == 0)
        return;
    ListTree* tree = static_cast<ListTree*>(currentTree());
    if (!tree)
        return;
    tree->unzoomAll();
    tree->zoom(itemId);
    tree->setFocus(Qt::MouseFocusReason);
}
