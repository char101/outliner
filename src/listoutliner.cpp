#include "listoutliner.h"
#include "sqlquery.h"

#include <QDebug>

ListOutliner::ListOutliner(QWidget* parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    tree = new ListOutlinerTree(this);
    tree->setHeaderHidden(true);
    layout->addWidget(tree);
}

void ListOutliner::loadOutline(int listId)
{
    if (listId == 0)
        return;
    tree->clear();
    loadOutlineInner(listId);
    tree->expandAll();
}

void ListOutliner::loadOutlineInner(int listId, int parentId, QTreeWidgetItem* parent)
{
    SqlQuery sql;
    sql.prepare(QString("SELECT id, content FROM list_item WHERE list_id = :list AND parent_id %0 AND is_project = 1 ORDER BY weight ASC").arg(parentId ? "= :parent" : "IS NULL"));
    sql.bindValue("list", listId);
    if (parentId)
        sql.bindValue("parent", parentId);
    if (!sql.exec())
        return;
    while (sql.next()) {
        int id = sql.value(0).toInt();
        QString content = sql.value(1).toString();
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(content));
        item->setData(0, Qt::UserRole, id);
        if (!parent)
            tree->addTopLevelItem(item);
        else
            parent->addChild(item);
        loadOutlineInner(listId, id, item);
    }
}

QSize ListOutliner::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    return QSize(100, size.height());
}
