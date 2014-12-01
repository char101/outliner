#pragma once

#include "constants.h"

#include <QAbstractItemModel>

class ListTree;
class ListItem;

class ListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ListModel(int listId, ListTree* parent = 0);
    ~ListModel();

    ListItem* root() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    ListItem* itemFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromItem(ListItem* item) const;
    QModelIndex indexFromId(int itemId) const;
    ListItem* itemFromId(int itemId, ListItem* parent = 0) const;

    QModelIndex appendAfter(const QModelIndex& index, QString content, App::AppendMode mode = App::AppendAfter);
    QModelIndex appendChild(const QModelIndex& index, QString content);

    void sort(ListItem* parent, App::SortMode mode);
    QModelIndex moveItemVertical(const QModelIndex& index, int direction);
    QModelIndex moveItemHorizontal(const QModelIndex& index, int direction);

    void removeItem(const QModelIndex& index);

    void itemChanged(ListItem* item, const QVector<int>& roles = QVector<int>());
private:
    int _listId;
    ListItem* _root;

    void _loadItems(ListItem* parent);
    QModelIndex _appendAfter(ListItem* item, const QString& content, App::AppendMode mode);
    bool _removeItem(ListItem* item);
};
