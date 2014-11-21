#include "mainwindow.h"

#include <QDesktopWidget>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSqlQuery>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow() : QMainWindow()
{
    menuVisible = false;
    statusBarVisible = false;

    setWindowTitle("Outliner");
    setWindowIcon(QIcon(":app.png"));

    setupMenu();
    setupStatusBar();
    setupUi();
}

void MainWindow::setupMenu()
{
    QMenuBar* menuBar = this->menuBar();
    if (!menuVisible)
        menuBar->hide();

    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction("Exit");
}

void MainWindow::setupUi()
{
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet("QTabWidget::pane { padding-bottom: 0 }");

    listWidget = new ListWidget(this);
    tabWidget->addTab(listWidget, QIcon(":list.png"), "&Lists");

    // auto calendarTab = new QWidget;
    // auto calendarLayout = new QVBoxLayout(calendarTab);
    // calendarLayout->setContentsMargins(0, 0, 0, 0);
    // tabWidget->addTab(calendarTab, QIcon(":calendar.png"), "&Schedule");
    //
    // auto pomodoroTab = new QWidget;
    // auto pomodoroLayout = new QVBoxLayout(pomodoroTab);
    // pomodoroLayout->setContentsMargins(0, 0, 0, 0);
    // tabWidget->addTab(pomodoroTab, QIcon(":timer.png"), "&Timer");

    setCentralWidget(tabWidget);
}

void MainWindow::setupStatusBar()
{
    QStatusBar* statusBar = this->statusBar();
    if (!statusBarVisible)
        statusBar->hide();
}

void MainWindow::setDatabasePath(const QString& dbPath)
{
    statusBar()->addWidget(new QLabel(dbPath));
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
    if (key->key() == Qt::Key_Escape) {
        close();
    }
}
