#pragma once

#include "listoutlinertree.h"
#include "markdownrenderer.h"

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

class ListOutliner : public QWidget
{
    Q_OBJECT
public:
    ListOutliner(QWidget* parent = 0);
    QSize sizeHint() const { return QSize(80, QWidget::sizeHint().height()); };
    ListOutlinerTree* tree() const { return _tree; };
public slots:
    void loadOutline(int listId);
    void reloadOutline();
private:
    QVBoxLayout* _layout{nullptr};
    ListOutlinerTree* _tree{nullptr};
    int _currentListId{0};

    static MarkdownRenderer _renderer;

    void _loadOutline(int parentId = 0, QTreeWidgetItem* parent = 0);
};
