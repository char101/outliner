#include "calendarwidget.h"

#include <QPainter>

CalendarWidget::CalendarWidget(QWidget* parent)
    : QCalendarWidget(parent), _currentDate(QDate::currentDate())
{
    _currentDatePen.setColor(Qt::green);
    _currentDateBrush.setColor(Qt::transparent);
}

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QCalendarWidget::paintCell(painter, rect, date);
    if (date == _currentDate) {
        painter->setPen(_currentDatePen);
        painter->setBrush(_currentDateBrush);
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
    }
}
