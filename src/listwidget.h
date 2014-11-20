#pragma once

#include <QWidget>
#include <QTabWidget>

class ListWidget : public QWidget
{
public:
    ListWidget(QWidget* parent = 0);
private:
    QTabWidget* tabWidget;
    QWidget* breadcrumb;
    void loadLists();
};
