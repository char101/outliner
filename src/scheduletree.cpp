#include "scheduletree.h"

ScheduleTree::ScheduleTree(QWidget* parent) : QTreeView(parent)
{
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);

    _delegate = new ScheduleItemDelegate(this);
    setItemDelegateForColumn(0, _delegate);

    _model = new ScheduleModel(this);
    setModel(_model);

    expandAll();
}
