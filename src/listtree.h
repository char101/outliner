#pragma once

#include <QTreeView>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

#include "htmldelegatetree.h"
#include "listmodel.h"
#include "listwidget.h"
#include "listitem.h"

class ListTree : public QTreeView
{
    Q_OBJECT
public:
    ListTree(int listId, QWidget* parent = 0);
    int listId() const { return _listId; };
    ListWidget* parent() const;
    void keyPressEvent(QKeyEvent* event);
    // void mousePressEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void contextMenuEvent(QContextMenuEvent* event);
    ListModel* model() const;
    void resizeEvent(QResizeEvent* event);
    void restoreExpandedState(const QModelIndex& index);
    void restoreExpandedState(QStandardItem* parent = 0);
    void remove(const QModelIndex& index);
    void edit(const QModelIndex& index);
    void zoom(const QModelIndex& index);
    void zoom(int itemId);
    void unzoom();
    void scrollTo(int itemId);
public slots:
    void unzoomTo(const QModelIndex& index);
    void unzoomAll();
signals:
    void zoomed(ListItem* item);
    void unzoomed();
private:
    int _listId;
    HtmlDelegateTree* itemDelegate;
    QTimer resizeTimer;
    void resizeDone();
    void resizeIndexes(QStandardItem* item);
    void appendItem(App::AppendMode mode);
    void moveVertical(int dir);
    void moveHorizontal(int dir);
};
