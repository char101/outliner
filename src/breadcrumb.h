#pragma once

#include <QToolBar>

class Breadcrumb : public QToolBar
{
public:
    Breadcrumb(QWidget* parent = 0);
    QAction* addAction(const QString& text);
    void addCurrent(const QString& text);
    void addSeparator();
    void popAction();
    void clear();
};
