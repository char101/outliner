#include "breadcrumb.h"

#include <QAction>
#include <QDebug>
#include <QLabel>

Breadcrumb::Breadcrumb(QWidget* parent) : QToolBar(parent)
{
    setStyleSheet("QToolButton { color:steelblue }");
}

QAction* Breadcrumb::addAction(const QString& text)
{
    setVisible(true);

    QAction* action = QToolBar::addAction(text);
    action->setObjectName("last");
    return action;
}

void Breadcrumb::addCurrent(const QString& text)
{
    setVisible(true);
    auto label = new QLabel(text);
    label->setStyleSheet("margin-left:5px");
    addWidget(label);
}

void Breadcrumb::addSeparator()
{
    auto sep = new QLabel("/");
    QAction* action = QToolBar::addWidget(sep);
    action->setDisabled(true); // must be disabled for popAction below
}

void Breadcrumb::popAction()
{
    QList<QAction*> actions = this->actions();
    if (actions.isEmpty())
        return;

    removeAction(actions.last());

    // remove separators
    for (int n = actions.length() - 2; n >= 0; --n) {
        if (actions[n]->isEnabled())
            break;
        removeAction(actions[n]);
    }

    // convert last action to current
    actions = this->actions();
    if (actions.isEmpty())
        return;
    QString text = actions.last()->text();
    removeAction(actions.last());
    addCurrent(text);

    actions = this->actions();
    if (actions.length() == 1) { // only the last Top item
        removeAction(actions[0]);
        setVisible(false);
    }
}

void Breadcrumb::clear()
{
    for (auto action : this->actions())
        removeAction(action);
}
