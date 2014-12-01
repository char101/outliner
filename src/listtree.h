#pragma once

#include <QTreeView>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

#include "listmodel.h"
#include "listwidget.h"
#include "listitem.h"
#include "listitemdelegate.h"

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
    void restoreExpandedState(ListItem* parent = 0);
    void remove(const QModelIndex& index);
    void edit(const QModelIndex& index);
    void editOrFocus(const QModelIndex& index);
    void zoom(const QModelIndex& index);
    void zoom(int itemId);
    void unzoom();
    void scrollTo(int itemId);

    ListItem* currentItem() const;
public slots:
    void unzoomTo(const QModelIndex& index);
    void unzoomAll();
signals:
    void zoomed(ListItem* item);
    void unzoomed();
    void itemIsProject(ListItem* item);
private:
    int _listId;
    ListItemDelegate* itemDelegate;
    QTimer resizeTimer;
    void resizeDone();
    void resizeIndexes(ListItem* item);
    void appendItem(App::AppendMode mode);
    void moveVertical(int dir);
    void moveHorizontal(int dir);
    bool _itemKeyPress(ListItem* item, int key, Qt::KeyboardModifiers modifiers);
};
