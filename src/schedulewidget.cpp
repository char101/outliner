#include "schedulewidget.h"

ScheduleWidget::ScheduleWidget(QWidget* parent) : QWidget(parent)
{
    _tree = new ScheduleTree(this);

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_tree);
}
