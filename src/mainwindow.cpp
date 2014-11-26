#include "mainwindow.h"
#include "utils.h"
#include "listoutliner.h"
#include "listmodel.h"
#include "listtree.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSqlQuery>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDockWidget>

MainWindow::MainWindow() : QMainWindow()
{
    menuVisible = true;
    statusBarVisible = true;

    setWindowTitle("Outliner");
    setWindowIcon(Util::findIcon("app"));

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
    tabWidget->tabBar()->hide();

    listWidget = new ListWidget(this);
    tabWidget->addTab(listWidget, Util::findIcon("list"), "&Lists");

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

    QDockWidget* leftDock = new QDockWidget("Outline", this);
    // QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    listOutliner = new ListOutliner(this);
    leftDock->setWidget(listOutliner);
    // leftDock->setLayout(new QHBoxLayout());
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    QDockWidget* rightDock = new QDockWidget("Schedule", this);
    addDockWidget(Qt::LeftDockWidgetArea, rightDock);

    tabifyDockWidget(leftDock, rightDock);
    leftDock->raise();

    connect(listWidget, &ListWidget::listSelected, listOutliner, &ListOutliner::loadOutline);
    connect(listOutliner->tree, &ListOutlinerTree::itemClickedAlt, [this](QTreeWidgetItem* item, int column) {
        if (column == 0)
            listWidget->scrollTo(item->data(0, Qt::UserRole).toInt());
    });
    connect(listOutliner->tree, &ListOutlinerTree::itemDoubleClickedAlt, [this](QTreeWidgetItem* item, int column) {
        if (column == 0)
            listWidget->zoomTo(item->data(0, Qt::UserRole).toInt());
    });

    listWidget->loadLists();
    for (auto tree : listWidget->trees()) {
        connect(static_cast<ListTree*>(tree)->model(), &ListModel::itemIsProject, [this](QStandardItem* item) {
            Q_UNUSED(item);
            listOutliner->loadOutline(listWidget->currentListId());
        });
    }
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
