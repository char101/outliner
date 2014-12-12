#include "listitemeditdialog.h"
#include "sqlquery.h"
#include "constants.h"
#include "utils.h"

#include <QPushButton>
#include <QTabBar>
#include <QDebug>
#include <QTimer>
#include <QFormLayout>
#include <QMessageBox>

ListItemEditDialog::ListItemEditDialog(QWidget* parent, const QString& title) : QDialog(parent)
{
    setWindowIcon(QIcon(":app.png"));
    if (!title.isNull())
        setWindowTitle(title);

    _calendarWidget = new CalendarWidget(this);

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);

    _tabWidget = new QTabWidget(this);
    _layout->addWidget(_tabWidget);

    _setupSimpleEditorTab();
    _setupTextEditorTab();

    _setupAttributes();

    _setupButtons();

    _simpleEditor->setFocus(Qt::MouseFocusReason);
}

void ListItemEditDialog::_setupSimpleEditorTab()
{
    auto widget = new QWidget;
    auto widgetLayout = new QVBoxLayout(widget);

    _simpleEditor = new QLineEdit();
    widgetLayout->addWidget(_simpleEditor);

    _tabWidget->addTab(widget, "Simple Editor");
}

void ListItemEditDialog::_setupTextEditorTab()
{
    _textEditor = new QPlainTextEdit(this);
    _tabWidget->addTab(_textEditor, "Text Editor");
}

void ListItemEditDialog::_setupAttributes()
{
    _dueDateEdit = new DateEdit(this);
    _dueDateEdit->setDisplayFormat("dd MMM yyyy");
    _dueDateEdit->setCalendarPopup(true);
    _dueDateEdit->setCalendarWidget(_calendarWidget);
    _dueDateEdit->setDate(QDate::currentDate());
    _dueDateEdit->setMinimumDate(QDate::currentDate());

    _dueDateCheck = new QCheckBox(this);

    auto layout = new QFormLayout();
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    auto dueDateLayout = new QHBoxLayout();
    dueDateLayout->setContentsMargins(0, 0, 0, 0);
    dueDateLayout->addWidget(_dueDateEdit);
    dueDateLayout->addWidget(_dueDateCheck);

    connect(_dueDateEdit, &QDateEdit::dateChanged, [this](const QDate& date) {
        Q_UNUSED(date);
        _dueDateCheck->setCheckState(Qt::Checked);
    });

    layout->addRow("Due Date", dueDateLayout);

    _layout->addLayout(layout);
}

void ListItemEditDialog::_setupButtons()
{
    auto buttonBoxLayout = new QHBoxLayout;
    buttonBoxLayout->setContentsMargins(5, 0, 5, 5);
    _layout->addLayout(buttonBoxLayout);

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(_buttonBox, &QDialogButtonBox::accepted, this, &ListItemEditDialog::accept);
    connect(_buttonBox, &QDialogButtonBox::rejected, this, &ListItemEditDialog::reject);
    buttonBoxLayout->addWidget(_buttonBox);
}

QSize ListItemEditDialog::sizeHint() const
{
    int width = 600;
    int height = _tabWidget->tabBar()->height() + _simpleEditor->height() + _buttonBox->height();

    return QSize(width, height);
}

void ListItemEditDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::ControlModifier) {
        accept();
        return;
    }
    QDialog::keyPressEvent(event);
}

void ListItemEditDialog::done(int r)
{
    if (r == QDialog::Accepted && text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Content is required");
        return;
    }
    QDialog::done(r);
}

QString ListItemEditDialog::text() const
{
    int tab = _tabWidget->currentIndex();
    QString text;
    if (tab == App::SimpleEditorTab)
        text = _simpleEditor->text();
    else
        text = _textEditor->toPlainText();
    return text.trimmed();
}

void ListItemEditDialog::setText(QString value)
{
    _textEditor->setPlainText(value);

    if (_isSimpleText(value)) {
        _simpleEditor->setText(value);
        if (!_tabWidget->isTabEnabled(App::SimpleEditorTab))
            _tabWidget->setTabEnabled(App::SimpleEditorTab, true);
        _simpleEditor->setFocus(Qt::MouseFocusReason);
    } else {
        _tabWidget->setTabEnabled(App::SimpleEditorTab, false);
    }
}

QDate ListItemEditDialog::dueDate() const
{
    QString text = this->text();
    if (text.contains('@')) {
        QString part = text.section('@', -1);
        QDate ret = Util::parseDate(part);
        if (ret.isValid())
            return ret;
    }

    if (_dueDateCheck->checkState() == Qt::Checked)
        return _dueDateEdit->date();

    return QDate();
}

void ListItemEditDialog::setDueDate(QDate date)
{
    if (date.isValid()) {
        _dueDateEdit->setDate(date);
        _dueDateCheck->setCheckState(Qt::Checked);
    }
}

bool ListItemEditDialog::_isSimpleText(const QString& text) const
{
    return text.indexOf('\n') == -1 && text.indexOf('\r') == -1;
}
