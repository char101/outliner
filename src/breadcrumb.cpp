#include "breadcrumb.h"

#include <QAction>
#include <QDebug>
#include <QLabel>

Breadcrumb::Breadcrumb(QWidget* parent) : QToolBar(parent)
{
    setStyleSheet("QToolButton { color:steelblue }");
    setIconSize(QSize(16, 16));
}

QAction* Breadcrumb::addAction(const QString& text)
{
    setVisible(true);

    QAction* action = QToolBar::addAction(text);
    action->setObjectName("last");
    return action;
}

QAction* Breadcrumb::addAction(const QIcon& icon, const QString& text)
{
    return QToolBar::addAction(icon, text);
}

void Breadcrumb::addCurrent(const QString& text)
{
    setVisible(true);
    auto label = new QLabel(text);
    label->setStyleSheet("margin-left:5px");
    addWidget(label);
}

void Breadcrumb::popAction()
{
    QList<QAction*> actions = this->actions();
    if (actions.isEmpty())
        return;

    qDebug() << QString("%0:%1 (%2)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__) << actions.last();
    removeAction(actions.last());

    // remove separators; 2 because the last one has been removed
    for (int n = actions.length() - 2; n >= 0; --n) {
        if (actions[n]->isSeparator())
            removeAction(actions[n]);
        break;
    }

    // convert last action to current
    actions = this->actions();
    if (actions.isEmpty())
        return;
    QString text = actions.last()->text();
    removeAction(actions.last());
    qDebug() << QString("%0:%1 (%2)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__) << text;
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
