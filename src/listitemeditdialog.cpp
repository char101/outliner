#include "listitemeditdialog.h"
#include "sqlquery.h"
#include "constants.h"

#include <QPushButton>
#include <QTabBar>
#include <QDebug>
#include <QTimer>

ListItemEditDialog::ListItemEditDialog(QWidget* parent) : QDialog(parent)
{
    setWindowIcon(QIcon(":app.png"));

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);

    setupSimpleEditorTab();
    setupTextEditorTab();

    setupButtons();

    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    simpleEditor->setFocus(Qt::MouseFocusReason);
}

void ListItemEditDialog::setupSimpleEditorTab()
{
    auto widget = new QWidget;
    auto widgetLayout = new QVBoxLayout(widget);

    simpleEditor = new QLineEdit();
    widgetLayout->addWidget(simpleEditor);

    tabWidget->addTab(widget, "Simple Editor");
}

void ListItemEditDialog::setupTextEditorTab()
{
    textEditor = new QPlainTextEdit(this);
    tabWidget->addTab(textEditor, "Text Editor");
}

void ListItemEditDialog::setupButtons()
{
    auto buttonBoxLayout = new QHBoxLayout;
    buttonBoxLayout->setContentsMargins(5, 0, 5, 5);
    layout->addLayout(buttonBoxLayout);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ListItemEditDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ListItemEditDialog::reject);
    buttonBoxLayout->addWidget(buttonBox);
}

QSize ListItemEditDialog::sizeHint() const
{
    int width = 600;
    int height = tabWidget->tabBar()->height() + simpleEditor->height() + buttonBox->height();

    return QSize(width, height);
}

QString ListItemEditDialog::text() const
{
    int tab = tabWidget->currentIndex();
    QString text;
    if (tab == App::SimpleEditorTab)
        text = simpleEditor->text();
    else
        text = textEditor->toPlainText();
    return text.trimmed();
}

QDateTime ListItemEditDialog::datetime() const
{
    return QDateTime::currentDateTime();
}

void ListItemEditDialog::setText(QString value)
{
    textEditor->setPlainText(value);

    if (isSimpleText(value)) {
        simpleEditor->setText(value);
        if (!tabWidget->isTabEnabled(App::SimpleEditorTab))
            tabWidget->setTabEnabled(App::SimpleEditorTab, true);
        simpleEditor->setFocus(Qt::MouseFocusReason);
    } else {
        tabWidget->setTabEnabled(App::SimpleEditorTab, false);
    }
}

void ListItemEditDialog::done(int r)
{
    if (r == QDialog::Accepted && text().isEmpty()) {
        statusBar->showMessage("Content is required");
        return;
    }
    QDialog::done(r);
}

void ListItemEditDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::ControlModifier) {
        accept();
        return;
    }
    QDialog::keyPressEvent(event);
}

bool ListItemEditDialog::isSimpleText(const QString& text) const
{
    return text.indexOf('\n') == -1 && text.indexOf('\r') == -1;
}
