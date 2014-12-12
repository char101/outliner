#include "aboutdialog.h"

#include "markdownrenderer.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QFile>
#include <QTextStream>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("About");
    setModal(true);

    QString info("<center>");
    info += "<h2>Taskware</h2>";
    info += "<p>Build date: " __DATE__ "</p>";
    info += "</center>";
    QLabel* appLabel = new QLabel(info);

    QTabWidget* tabs = new QTabWidget(this);
    tabs->addTab(_creditsPage(), "Credits");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(10);
    layout->addWidget(appLabel);
    layout->addSpacing(10);
    layout->addWidget(tabs);
}

QTextEdit* AboutDialog::_creditsPage()
{
    QTextEdit* editor = new QTextEdit(this);
    editor->setReadOnly(true);

    QFile credits(":CREDITS.md");
    if (credits.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&credits);
        QString contents = in.readAll();
        credits.close();

        MarkdownRenderer renderer;
        editor->setHtml(renderer.convert(contents));
    }

    return editor;
}
