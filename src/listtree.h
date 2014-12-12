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

    void keyPressEvent(QKeyEvent* event) override;
    // void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    ListModel* model() const;
    void restoreExpandedState(const QModelIndex& index);
    void restoreExpandedState(ListItem* parent = 0);
    void remove(const QModelIndex& index);
    void edit(const QModelIndex& index);
    void editOrFocus(const QModelIndex& index);

    void zoom(const QModelIndex& index);
    void zoom(int itemId);
    void unzoom();
    bool isZoomed() const;

    void scrollTo(int itemId);

    ListItem* currentItem() const;
    ListItem* rootItem() const;

    bool isHidingCompleted() const { return _isHidingCompleted; };
    void hideCompleted();
    void showCompleted();
public slots:
    void unzoomTo(const QModelIndex& index);
    void unzoomAll();
signals:
    void zoomed(ListItem* item);
    void unzoomed();
protected:
    void resizeEvent(QResizeEvent* event) override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
private:
    int _listId;
    ListItemDelegate* _itemDelegate;
    QTimer _resizeTimer;

    bool _isHidingCompleted{false};

    void _appendItem(App::AppendMode mode);
    void _moveVertical(int dir);
    void _moveHorizontal(int dir);
    bool _itemKeyPress(ListItem* item, int key, Qt::KeyboardModifiers modifiers);
    void _hideCompletedRows(ListItem* parent = 0);
    void _showCompletedRows(ListItem* parent = 0);

    void _drawPriorityBar(ListItem* item, QPainter* painter, const QRect& rect, int x, int gap = 0) const;
};
