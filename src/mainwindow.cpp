#include "mainwindow.h"

#include "aboutdialog.h"
#include "utils.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSqlQuery>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QToolTip>
#include <QApplication>

MainWindow::MainWindow() : QMainWindow()
{
    _menuVisible = true;
    _statusBarVisible = true;

#ifdef QT_DEBUG
    setWindowTitle("Task Manager [DEBUG]");
#else
    setWindowTitle("Task Manager");
#endif

    setWindowIcon(Util::findIcon("app", QList<int>{16}));

    _list = new ListWidget(this);
    _schedule = new ScheduleWidget(this);
    _statusBar = statusBar();
    _tabs = new QTabWidget(this);

    _setupUi();
    _setupDocks();

    _setupMenu();
    _setupStatusBar();
}

void MainWindow::_setupMenu()
{
    QMenuBar* menuBar = this->menuBar();
    if (!_menuVisible)
        menuBar->hide();

    QMenu* menu = nullptr;
    QAction* action = nullptr;

    menu = menuBar->addMenu("&File");
    menu->addAction("Exit", this, SLOT(close()));

    menu = menuBar->addMenu("&List");
    action = menu->addAction("&Hide Completed", _list, SLOT(toggleHideCompleted()));
    action->setCheckable(true);
    action->setChecked(true);

    menu->addSeparator();

    menu->addAction("&Expand All", _list, SLOT(expandAll()));
    menu->addAction("&Collapse All", _list, SLOT(collapseAll()));

    menu = menuBar->addMenu("&Help");
    menu->addAction("&About", this, SLOT(showAboutDialog()));
    menu->addAction("About &Qt", this, SLOT(showAboutQtDialog()));
}

void MainWindow::_setupUi()
{
    _list->loadLists();
    connect(_list, &ListWidget::operationError, [this](const QString& message) {
        _statusBar->showMessage(message, 5000);
    });
    connect(_list, &ListWidget::scheduleChanged, _schedule, &ScheduleWidget::reload);

    _tabs->setStyleSheet("QTabWidget::pane { padding-bottom: 0 }");
    _tabs->addTab(_list, Util::findIcon("list"), "&Tasks");
    _tabs->addTab(_schedule, Util::findIcon("schedule"), "&Schedule");
    setCentralWidget(_tabs);
}

void MainWindow::_setupDocks()
{
    // QDockWidget* leftDock = new QDockWidget("Outline", this);
    // QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    // listOutliner = new ListOutliner(this);
    // leftDock->setWidget(listOutliner);
    // leftDock->setLayout(new QHBoxLayout());
    // addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // QDockWidget* rightDock = new QDockWidget("Schedule", this);
    // addDockWidget(Qt::LeftDockWidgetArea, rightDock);

    // tabifyDockWidget(leftDock, rightDock);
    // leftDock->raise();
}

void MainWindow::_setupStatusBar()
{
    if (!_statusBarVisible)
        _statusBar->hide();
}

void MainWindow::setDatabasePath(const QString& dbPath)
{
    _statusBar->addPermanentWidget(new QLabel(dbPath));
}

QSize MainWindow::sizeHint() const
{
    QDesktopWidget widget;
    QRect primaryScreenSize = widget.availableGeometry(widget.primaryScreen());
    const int width = qMin(640, primaryScreenSize.width());
    const int height = primaryScreenSize.height();
    return QSize(width, height);
}

void MainWindow::keyPressEvent(QKeyEvent* key)
{
    if (key->key() == Qt::Key_Escape)
        close();
    QMainWindow::keyPressEvent(key);
}

void MainWindow::showAboutDialog()
{
    AboutDialog aboutDialog;
    aboutDialog.exec();
}

void MainWindow::showAboutQtDialog()
{
    QApplication::aboutQt();
}
