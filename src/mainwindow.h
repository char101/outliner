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
    bool _menuVisible{false};
    bool _statusBarVisible{false};

    QTabWidget* _tabs{nullptr};
    ListWidget* _list{nullptr};
    ScheduleWidget* _schedule{nullptr};
    QStatusBar* _statusBar{nullptr};

    void _setupMenu();
    void _setupStatusBar();
    void _setupUi();
    void _setupDocks();
};
