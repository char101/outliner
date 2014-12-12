#pragma once

#include <memory>

#include <QDateEdit>
#include <QCalendarWidget>
#include <QKeyEvent>
#include <QDebug>

class DateEdit : public QDateEdit
{
public:
    DateEdit(QWidget* parent = 0) : QDateEdit(parent)
    {
        setCalendarPopup(true);
    };

    void focusInEvent(QFocusEvent *event) override
    {
        qDebug() << __FILE__ << __LINE__ << __FUNCTION__;

        // calendarWidget()->show();

        auto keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Select, Qt::NoModifier);
        keyPressEvent(keyEvent);
        delete keyEvent;

        QDateEdit::focusInEvent(event);
    };
};
