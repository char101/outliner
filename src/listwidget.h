#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QVBoxLayout>

class ListTree;
class ListOutliner;

class ListWidget : public QWidget
{
    Q_OBJECT
public:
    ListWidget(QWidget* parent = 0);
    void loadLists();
    ListTree* currentTree() const;
    int currentListId() const;
    QList<ListTree*> trees() const;
    bool isZoomed() const;
public slots:
    void scrollTo(int itemId);
    void zoomTo(int itemId);
    void toggleHideCompleted();
    void expandAll();
    void collapseAll();
signals:
    void listSelected(int listId);
    void scheduleChanged();
    void operationError(const QString& message);
private:
    QVBoxLayout* _layout;
    QSplitter* _splitter;
    ListOutliner* _outliner;
    QTabWidget* _tabWidget;
    QList<ListTree*> _trees;
};
