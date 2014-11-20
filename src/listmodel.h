#pragma once

#include <QStandardItemModel>

#include "constants.h"
#include "markdownrenderer.h"

class ListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    ListModel(int listId, QObject* parent = 0);
    QModelIndex appendAfter(const QModelIndex& index, QString text = "New item");
    QModelIndex appendChild(const QModelIndex& index, QString text = "New item");
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onItemChanged(QStandardItem* item);
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    void removeItem(const QModelIndex& index);
    bool removeItem(QStandardItem* item);
    void setItemCheckable(const QModelIndex& index, bool checkable);
    void toggleItemCheckable(const QModelIndex& index);
    void toggleItemCheckState(const QModelIndex& index);
    void setItemChecked(QStandardItem* item, bool checked);
    void toggleHighlight(const QModelIndex& index);
    QModelIndex moveItemVertical(const QModelIndex& index, int direction);
    QStandardItem* moveItemHorizontal(QStandardItem* item, int direction);
    void setExpandedState(QStandardItem* ite, bool expanded);
    void setExpandedState(const QModelIndex& index, bool expanded);
private:
    int listId;
    void loadItems(QStandardItem* parent, int parentId = 0);
};
