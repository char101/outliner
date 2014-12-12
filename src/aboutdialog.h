#pragma once

#include <QDialog>
#include <QTextEdit>

class AboutDialog : public QDialog
{
public:
    AboutDialog(QWidget*  parent = 0);
private:
    QTextEdit* _creditsPage();
};
