#pragma once

#include "scheduleitem.h"

#include <QAbstractItemModel>

class ScheduleTree;

class ScheduleModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ScheduleModel(QObject* parent = 0);
    ~ScheduleModel();

    ScheduleItem* root() const { return _root; };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    };

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    ScheduleItem* itemFromIndex(const QModelIndex& index) const { return index.isValid() ? static_cast<ScheduleItem*>(index.internalPointer()) : _root; };
    QModelIndex indexFromItem(ScheduleItem* item) const { return item->isRoot() ? QModelIndex() : createIndex(item->row(), 0, item); };

    void clear();
public slots:
    void reload();
private:
    ScheduleItem* _root{nullptr};

    void _loadItems();
    QVariant _dataContent(ScheduleItem* item, int role) const;
    bool _setDataContent(ScheduleItem* item, const QVariant& value, int role);
};
