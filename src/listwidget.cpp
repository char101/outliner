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

    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet("QTabWidget::pane { padding-bottom: 0 }");
    layout->addWidget(tabWidget);

    loadLists();
}

void ListWidget::loadLists()
{
    QSqlQuery sql("SELECT id, name FROM list ORDER BY weight ASC");
    int nlist = 0;
    while (sql.next()) {
        ++nlist;
        int listId = sql.value(0).toInt();

        auto tree = new ListTree(listId, this);

        auto breadcrumb = new Breadcrumb(this);
        breadcrumb->setVisible(false);

        connect(tree, &ListTree::zoomed, [breadcrumb, tree](ListItem* item) {
            // collect all items from this item to the root
            QList<ListItem*> items;
            for (ListItem* curr = item; curr; curr = curr->parent())
                items << curr;

            breadcrumb->clear();

            // root item
            QAction* action = breadcrumb->addAction("Top");
            connect(action, &QAction::triggered, [=]() {
                tree->unzoomAll();
            });
            breadcrumb->addSeparator();

            // add button for each item to the breadcrumb
            for (int i = items.length() - 1; i > 0; --i) {
                QAction* action = breadcrumb->addAction(items[i]->label());
                if (i > 0)
                    breadcrumb->addSeparator();
                connect(action, &QAction::triggered, [=]() {
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

        tabWidget->addTab(widget, sql.value(1).toString());
    }

    if (nlist == 1)
        tabWidget->tabBar()->hide();
}
