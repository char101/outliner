#pragma once

#include "schedulemodel.h"
#include "scheduleitemdelegate.h"

#include <QTreeView>

class ScheduleTree : public QTreeView
{
    Q_OBJECT
public:
    class ScheduleTree(QWidget* parent);
    ScheduleModel* model() const { return _model; };
public slots:
    void reload()
    {
        _model->reload();
        expandAll();
    }
private:
    ScheduleModel* _model;
    ScheduleItemDelegate* _delegate;
};
