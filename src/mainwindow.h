#pragma once

#include <QMainWindow>
#include <QKeyEvent>
#include <QTabWidget>
#include <QStatusBar>

#include "listwidget.h"
#include "schedulewidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    QSize sizeHint() const;
    void keyPressEvent(QKeyEvent* key);
    void setDatabasePath(const QString& dbPath);
public slots:
    void showAboutDialog();
    void showAboutQtDialog();
private:
    bool _menuVisible;
    bool _statusBarVisible;

    QTabWidget* _tabs;
    ListWidget* _list;
    ScheduleWidget* _schedule;
    QStatusBar* _statusBar;

    void _setupMenu();
    void _setupStatusBar();
    void _setupUi();
    void _setupDocks();
};
