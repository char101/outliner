#pragma once

#include "dateedit.h"
#include "calendarwidget.h"

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QStatusBar>
#include <QDate>
#include <QDateEdit>
#include <QCheckBox>

class ListItemEditDialog : public QDialog
{
    Q_OBJECT
public:
    ListItemEditDialog(QWidget* parent = 0, const QString& title = QString());
    QSize sizeHint() const;
    void keyPressEvent(QKeyEvent* event);
    void done(int r);

    QString text() const;
    void setText(QString value);

    QDate dueDate() const;
    void setDueDate(QDate date);
private:
    QVBoxLayout* _layout;
    QTabWidget* _tabWidget;
    QLineEdit* _simpleEditor;
    QPlainTextEdit* _textEditor;

    DateEdit* _dueDateEdit;
    QCheckBox* _dueDateCheck;
    CalendarWidget* _calendarWidget;

    QDialogButtonBox* _buttonBox;

    bool _isModified{false};

    void _setupSimpleEditorTab();
    void _setupTextEditorTab();
    void _setupAttributes();
    void _setupButtons();
    bool _isSimpleText(const QString& text) const;
};
