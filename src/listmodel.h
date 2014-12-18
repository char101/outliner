#pragma once

#include "constants.h"
#include "listitem.h"

#include <QAbstractItemModel>

class ListTree;

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

    ListItem* itemFromIndex(const QModelIndex& index) const { return index.isValid() ? static_cast<ListItem*>(index.internalPointer()) : _root; };
    QModelIndex indexFromItem(ListItem* item) const { return item->isRoot() ? QModelIndex() : createIndex(item->row(), 0, item); };
    QModelIndex indexFromId(int itemId) const;
    ListItem* itemFromId(int itemId, ListItem* parent = 0) const;

    QModelIndex appendChild(const QModelIndex& parent, int row, QString content);
    QModelIndex appendAfter(const QModelIndex& index, QString content, App::AppendMode mode = App::AppendAfter);

    void sort(ListItem* parent, App::SortMode mode);
    QModelIndex moveItemVertical(const QModelIndex& index, App::Direction direction);
    QModelIndex moveItemHorizontal(const QModelIndex& index, int direction);

    void removeItem(const QModelIndex& index);

    void itemChanged(ListItem* item, const QVector<int>& roles = QVector<int>());

    static bool isNewItemCheckable(ListItem* parent, int row = 0);
signals:
    void projectAdded(ListItem* item);
    void projectChanged(ListItem* item);
    void projectRemoved();
    void scheduleChanged();
    void operationError(const QString& message);
private:
    int _listId{0};
    ListItem* _root{nullptr};

    void _loadItems(ListItem* parent);
    QModelIndex _appendAfter(ListItem* item, const QString& content, App::AppendMode mode);
    bool _removeItem(ListItem* item);
};
