#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QStatusBar>

class ListItemEditDialog : public QDialog
{
    Q_OBJECT
public:
    ListItemEditDialog(QWidget* parent = 0);
    QSize sizeHint() const;
    QString text() const;
    void setText(QString value);
    void keyPressEvent(QKeyEvent* event);
    void done(int r);
private:
    QVBoxLayout* layout;
    QTabWidget* tabWidget;
    QLineEdit* simpleEditor;
    QPlainTextEdit* textEditor;
    QDialogButtonBox* buttonBox;
    QStatusBar* statusBar;
    bool isModified;
    void setupSimpleEditorTab();
    void setupTextEditorTab();
    void setupButtons();
    bool isSimpleText(const QString& text) const;
};
