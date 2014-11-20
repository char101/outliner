#pragma once

#include <QMainWindow>
#include <QKeyEvent>
#include <QTabWidget>

#include "listwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    QSize sizeHint() const;
    void keyPressEvent(QKeyEvent* key);
    void setDatabasePath(const QString& dbPath);
private:
    QTabWidget* tabWidget;
    ListWidget* listWidget;
    bool menuVisible;
    bool statusBarVisible;
    void setupMenu();
    void setupStatusBar();
    void setupUi();
};
