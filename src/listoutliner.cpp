#include "listoutliner.h"
#include "sqlquery.h"

#include <QDebug>

MarkdownRenderer ListOutliner::_renderer{};

ListOutliner::ListOutliner(QWidget* parent) : QWidget(parent)
{
    _tree = new ListOutlinerTree(this);

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_tree);
}

void ListOutliner::loadOutline(int listId)
{
    if (listId == 0)
        return;

    _currentListId = listId;

    _tree->clear();
    _loadOutline();
    _tree->expandAll();
}

void ListOutliner::reloadOutline()
{
    _tree->clear();
    _loadOutline();
    _tree->expandAll();
}

void ListOutliner::_loadOutline(int parentId, QTreeWidgetItem* parent)
{
    SqlQuery sql;
    sql.prepare("SELECT id, content FROM list_item WHERE list_id = :list AND parent_id = :parent AND (is_project = 1 OR is_milestone = 1) ORDER BY weight ASC");
    sql.bindValue(":list", _currentListId);
    sql.bindValue(":parent", parentId);
    if (!sql.exec())
        return;
    while (sql.next()) {
        int c = -1;

        int id = sql.value(++c).toInt();
        QString content = sql.value(++c).toString();
        QString markdown = _renderer.convert(content);

        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(markdown));
        item->setData(0, Qt::UserRole, id);
        if (!parent)
            _tree->addTopLevelItem(item);
        else
            parent->addChild(item);
        _loadOutline(id, item);
    }
}
