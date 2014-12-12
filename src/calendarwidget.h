#pragma once

#include <QCalendarWidget>
#include <QPen>
#include <QBrush>

class CalendarWidget : public QCalendarWidget
{
public:
    CalendarWidget(QWidget* parent = 0);
protected:
    virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;
private:
    QDate _currentDate;
    QPen _currentDatePen;
    QBrush _currentDateBrush;
};
