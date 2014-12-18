#include "listitem.h"

#include "listmodel.h"
#include "sqlquery.h"
#include "utils.h"
#include "debug.h"

#include <QTextDocument>

const MarkdownRenderer ListItem::renderer{};

ListItem::ListItem(ListModel* model, int listId) : QObject(), _listId(listId), _isRoot(true)
{
    setModel(model);
}

ListItem::ListItem(int listId, int id, QString content)
    : QObject(), _listId(listId), _id(id), _markdown(content)
{
    _setMarkdown(content);
}

ListItem::ListItem(int listId, int id, QString content, bool isExpanded, bool isProject,
                   bool isMilestone, bool isHighlighted, bool isCheckable, bool isCompleted,
                   bool isCancelled, QDate dueDate, int priority)
    : QObject(), _listId(listId), _id(id)
{
    _setMarkdown(content);
    _setCheckable(isCheckable);

    _isExpanded = isExpanded;
    _isProject = isProject;
    _isMilestone = isMilestone;
    _isHighlighted = isHighlighted;
    if (_isCheckable) {
        _isCompleted = isCompleted;
        _isCancelled = isCancelled;
    }
    _priority = priority;
    _dueDate = dueDate;
}

ListItem::~ListItem() { qDeleteAll(_children); }

void ListItem::setModel(ListModel* model)
{
    if (_model == model)
        return;

    if (_model) {
        disconnect(this, &ListItem::scheduleChanged, _model, &ListModel::scheduleChanged);
        disconnect(this, &ListItem::operationError, _model, &ListModel::operationError);
    }

    _model = model;

    if (_model) {
        connect(this, &ListItem::scheduleChanged, _model, &ListModel::scheduleChanged);
        connect(this, &ListItem::operationError, _model, &ListModel::operationError);
    }
}

bool ListItem::sort(App::SortMode mode)
{
    int count = _children.count();

    QList<ListItem*> newChildren;
    for (int i = 0; i < count; ++i)
        newChildren << _children.at(i);

    if (mode == App::SortByStatus)
        std::stable_sort(newChildren.begin(), newChildren.end(), [](ListItem* lhs, ListItem* rhs) {
            return (lhs->weight() - rhs->weight()) < 0;
        });
    else if (mode == App::SortByStatusAndContent)
        std::stable_sort(newChildren.begin(), newChildren.end(), [](ListItem* lhs, ListItem* rhs) {
            int d = lhs->weight() - rhs->weight();
            if (d == 0)
                d = lhs->text().compare(rhs->text());
            return d < 0;
        });

    QSqlDatabase db = QSqlDatabase::database();
    bool localTransaction = db.transaction();
    bool success = true;

    for (int i = 0; i < count; ++i) {
        ListItem* child = newChildren.at(i);
        if (child->row() != i)
            success = child->setRow(i);
        if (!success)
            break;
    }

    if (success)
        _children = std::move(newChildren);

    if (localTransaction)
        if (success)
            db.commit();
        else
            db.rollback();

    return success;
}

int ListItem::weight() const
{
    if (_isCancelled)
        return 100;
    if (_isCompleted)
        return 99;
    if (_priority == 0)
        return 98;
    return _priority;
}

QString ListItem::html() const
{
    if (isProject())
        return QStringLiteral("<b>") + _html + QStringLiteral("</b>");
    return _html;
}

void ListItem::setFlag(Qt::ItemFlags flag, bool state)
{
    if (state)
        _flags |= flag;
    else
        _flags &= ~flag;
}

ListItem* ListItem::child(int row) const
{
    if (!(row >= 0 && row < _children.length()))
        return nullptr;
    return _children.at(row);
}

ListItem* ListItem::firstChild() const
{
    if (_children.isEmpty())
        return nullptr;
    return _children.first();
}

ListItem* ListItem::lastChild() const
{
    if (_children.isEmpty())
        return nullptr;
    return _children.last();
}

void ListItem::appendChild(ListItem* child)
{
    child->setParent(this, _children.length());
    _children.append(child);
}

void ListItem::insertChild(int row, ListItem* child)
{
    for (int i = row, n = _children.length(); i < n; ++i)
        _children.at(i)->_adjustRow(1);
    child->setParent(this, row);
    _children.insert(row, child);
}

void ListItem::removeChild(int row)
{
    int nchild = _children.length();
    if (!(row >= 0 && row < nchild))
        return;

    for (int i = row + 1; i < nchild; ++i)
        _children.at(i)->_adjustRow(-1);

    ListItem* item = _children.takeAt(row);
    delete item;
}

void ListItem::moveChild(int row) { _children.move(row, row + 1); }

bool ListItem::takeChildDb(int row)
{
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET weight = weight - 1 WHERE list_id = :list AND parent_id = "
                ":parent AND weight > :row");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", _id);
    sql.bindValue(":row", row);
    return sql.exec();
}

ListItem* ListItem::takeChild(int row)
{
    int nchild = _children.length();
    if (!(row >= 0 && row < nchild))
        return nullptr;

    for (int i = row + 1; i < nchild; ++i)
        _children.at(i)->_adjustRow(-1);

    ListItem* child = _children.takeAt(row);
    child->setParent(nullptr, 0);
    return child;
}

bool ListItem::setRow(int row)
{
    return _row == row || _setAttribute("weight", row) && (_row = row, true);
}

void ListItem::_setMarkdown(const QString& value)
{
    _markdown = value;
    _html = renderer.convert(value);

    QTextDocument doc;
    doc.setHtml(_html);
    _text = doc.toPlainText();

    _label = _text.replace('\n', " ").replace('\r', " ");
    if (_label.length() > 80) {
        _label.truncate(40);
        _label = _label.trimmed() + "...";
    }
}

bool ListItem::setMarkdown(const QString& markdown)
{
    return markdown == _markdown || _setAttribute("content", markdown) && (_setMarkdown(markdown), true);
}

void ListItem::setParent(ListItem* parent, int row)
{
    _parent = parent;
    if (parent) {
        setLevel(parent->level() + 1);
        setRow(row);
        setModel(parent->model());
    } else {
        setLevel(0);
        setRow(0);
        setModel(nullptr);
    }
}

bool ListItem::setParentDb(ListItem* parent, int row)
{
    if (parent == _parent && row == _row)
        return true;

    QSqlDatabase db = QSqlDatabase::database();
    bool localTransaction = db.transaction();
    bool success = true;

    SqlQuery sql;

    int parentId = parent->id();

    sql.prepare("UPDATE list_item SET weight = weight + 1 WHERE list_id = :list AND parent_id = "
                ":parent AND weight >= :row");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", parentId);
    sql.bindValue(":row", row);
    if (!sql.exec())
        success = false;
    else {
        sql.prepare("UPDATE list_item SET parent_id = :parent, weight = :row WHERE id = :id");
        sql.bindValue(":parent", parentId);
        sql.bindValue(":row", row);
        sql.bindValue(":id", _id);
        if (!sql.exec())
            success = false;
    }

    if (localTransaction)
        if (success)
            db.commit();
        else
            db.rollback();

    return success;
}

void ListItem::setLevel(int level)
{
    _level = level;
    for (ListItem* child : _children)
        child->setLevel(level + 1);
}

bool ListItem::setExpanded(bool isExpanded)
{
    return isExpanded == _isExpanded ||
           _setAttribute("is_expanded", isExpanded ? 1 : 0) && (_isExpanded = isExpanded, true);
}

void ListItem::_setCheckable(const bool isCheckable)
{
    _isCheckable = isCheckable;
    setFlag(Qt::ItemIsUserCheckable, isCheckable);
    if (!isCheckable) {
        if (isCompleted())
            setCompleted(false);
        if (isCancelled())
            setCancelled(false);
    }
}

bool ListItem::setCheckable(const bool isCheckable)
{
    if (isCheckable == _isCheckable)
        return true;

    QSqlDatabase db = QSqlDatabase::database();
    bool localTrans = db.transaction();

    bool success = true;
    if (_setAttribute("is_checkable", isCheckable ? 1 : 0)) {
        _setCheckable(isCheckable);
        if (!isCheckable) {
            if (isCompleted())
                success = setCompleted(false);
            if (success)
                if (isCancelled())
                    success = setCancelled(false);
        }
    } else
        success = false;

    if (localTrans)
        success ? db.commit() : db.rollback();
    return success;
}

bool ListItem::setCompleted(const bool isCompleted)
{
    if (!_isCheckable || _isCancelled)
        return false;
    return isCompleted == _isCompleted ||
           _setAttribute("is_completed", isCompleted ? 1 : 0) && (_isCompleted = isCompleted, true);
}

bool ListItem::setCancelled(const bool isCancelled)
{
    if (!_isCheckable || _isCompleted)
        return false;
    return isCancelled == _isCancelled ||
           _setAttribute("is_cancelled", isCancelled) && (_isCancelled = isCancelled, true);
}

bool ListItem::setProject(const bool isProject)
{
    if (isProject) {
        if (_isMilestone) {
            emit operationError("Cannot set milestone as project");
            return false;
        }
        if (!_parent) {
            emit operationError("This item has no parent");
            return false;
        }
        if (_parent->isMilestone()) {
            emit operationError("A project cannot become a child of a milestone");
            return false;
        }
        if (!(_parent->isRoot() || _parent->isProject())) {
            emit operationError("Only a top level item or a direct child of another project can be set as a projet");
            return false;
        }
        if (_isCheckable) {
            emit operationError("Cannot set a checkable item as a project");
            return false;
        }
    } else {
        for (auto child : _children)
            if (child->isProject() || child->isMilestone()) {
                emit operationError("Cannot unset project: the project has a child marked as project or milestone");
                return false;
            }
    }

    if (isProject == _isProject)
        return true;

    if (_setAttribute("is_project", isProject)) {
        _isProject = isProject;
        if (isProject)
            emit _model->projectAdded(this);
        else
            emit _model->projectRemoved();
        return true;
    }
    return false;
}

bool ListItem::setMilestone(const bool isMilestone)
{
    if (isMilestone) {
        if (_isProject) {
            emit operationError("Cannot set project as milestone");
            return false;
        }
        if (!_parent || !_parent->isProject()) {
            emit operationError("Milestone should be child of a project");
            return false;
        }
        if (_isCheckable) {
            emit operationError("Cannot set a checkable item as milestone");
            return false;
        }
    }
    if (isMilestone == _isMilestone)
        return true;
    if (_setAttribute("is_milestone", isMilestone)) {
        _isMilestone = isMilestone;
        if (_model)
            emit isMilestone ? _model->projectAdded(this) : _model->projectRemoved();
        return true;
    }
    return false;
}

bool ListItem::setHighlighted(const bool isHighlighted)
{
    return isHighlighted == _isHighlighted || _setAttribute("is_highlighted", isHighlighted) && (_isHighlighted = isHighlighted, true);
}

bool ListItem::setDueDate(const QDate& dueDate)
{
    if (dueDate == _dueDate)
        return true;
    if (_setAttribute("due_date", dueDate.isValid() ? dueDate.toString(Qt::ISODate) : QVariant())) {
        _dueDate = dueDate;
        emit scheduleChanged();
        return true;
    }
    return false;
}

bool ListItem::setPriority(int priority)
{
    if (_isMilestone) // milestone is ordered by date not priority
        return false;
    return priority == _priority || _setAttribute("priority", priority) && (_priority = priority, true);
}

bool ListItem::_setAttribute(const QString& column, QVariant value) const
{
    QString sql;
    if (column == "content")
        sql = QString("UPDATE list_item SET %1 = :%1, modified_at = CURRENT_TIMESTAMP WHERE id = :id").arg(column);
    else if (column == "is_completed" || column == "is_cancelled")
        sql = QString("UPDATE list_item SET %1 = :%1, completed_at = CURRENT_TIMESTAMP WHERE id = :id").arg(column);
    else
        sql = QString("UPDATE list_item SET %1 = :%1 WHERE id = :id").arg(column);

    SqlQuery q;
    q.prepare(sql);
    q.bindValue(":id", _id);
    q.bindValue(QStringLiteral(":") + column, value);
    return q.exec();
}
