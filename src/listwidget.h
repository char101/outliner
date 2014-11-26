#pragma once

#include <QWidget>
#include <QTabWidget>

class ListTree;

class ListWidget : public QWidget
{
    Q_OBJECT
public:
    ListWidget(QWidget* parent = 0);
    void loadLists();
    ListTree* currentTree() const;
    int currentListId() const;
    QList<ListTree*> trees() const;
public slots:
    void scrollTo(int itemId);
    void zoomTo(int itemId);
signals:
    void listSelected(int listId);
private:
    QTabWidget* _tabWidget;
    QList<ListTree*> _trees;
};
