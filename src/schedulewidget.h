#pragma once

#include "scheduletree.h"

#include <QWidget>
#include <QVBoxLayout>

class ScheduleWidget : public QWidget
{
    Q_OBJECT
public:
    ScheduleWidget(QWidget* parent = 0);
public slots:
    void reload() { _tree->reload(); };
private:
    QVBoxLayout* _layout{nullptr};
    ScheduleTree* _tree{nullptr};
};
