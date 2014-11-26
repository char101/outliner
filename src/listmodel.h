#pragma once

#include <QStandardItemModel>

#include "constants.h"
#include "markdownrenderer.h"

class ListItem;

class ListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    ListModel(int listId, QObject* parent = 0);
    QModelIndex appendAfter(const QModelIndex& index, QString text = "New item", App::AppendMode mode = App::AppendAfter);
    QModelIndex appendChild(const QModelIndex& index, QString text = "New item");
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onItemChanged(QStandardItem* item);
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    void removeItem(const QModelIndex& index);
    bool removeItem(QStandardItem* item);
    void toggleItemCheckable(const QModelIndex& index);
    void toggleItemCheckState(const QModelIndex& index, Qt::CheckState checkedState);
    void toggleItemChecked(const QModelIndex& index);
    void toggleItemCancelled(const QModelIndex& index);
    void toggleItemIsProject(const QModelIndex& index);
    void toggleHighlight(const QModelIndex& index);
    void setItemChecked(QStandardItem* item, bool checked);
    QModelIndex moveItemVertical(const QModelIndex& index, int direction);
    QStandardItem* moveItemHorizontal(QStandardItem* item, int direction);
    void setExpandedState(QStandardItem* ite, bool expanded);
    void setExpandedState(const QModelIndex& index, bool expanded);
    void sortChildren(QStandardItem* item);
    void sortCompleted(QStandardItem* item);
    void saveItemWeights(QStandardItem* parent);
    App::SortMode sortMode() { return _sortMode; };
    void setSortMode(App::SortMode mode) { _sortMode = mode; };

    QModelIndex indexFromId(int itemId) const;
    QStandardItem* itemFromId(int itemId, QStandardItem* parent = 0) const;
signals:
    void itemIsProject(QStandardItem* item);
private:
    int listId;
    App::SortMode _sortMode;
    void _loadItems(QStandardItem* parent, int parentId = 0);
    bool _saveItemWeights(QStandardItem* parent);
    bool _setItemIsProject(ListItem* item, bool isProject);
};
