#include "listitem.h"

#include "sqlquery.h"

#include <QTextDocument>
#include <QDebug>

const MarkdownRenderer ListItem::renderer{};

ListItem::ListItem(int listId) : _listId(listId) {}

ListItem::ListItem(int listId, int id, QString content)
    : _listId(listId), _id(id), _markdown(content)
{
    setMarkdown(content);
}

ListItem::ListItem(int listId, int id, QString content, bool isExpanded, bool isProject,
                   bool isHighlighted, bool isCheckable, bool isCompleted, bool isCancelled)
    : _listId(listId), _id(id)
{
    setMarkdown(content);
    setExpanded(isExpanded);
    setProject(isProject);
    setHighlighted(isHighlighted);
    setCheckable(isCheckable);
    setCompleted(isCompleted);
    if (isCheckable) {
        setCompleted(isCompleted);
        setCancelled(isCancelled);
    }
}

ListItem::~ListItem() { qDeleteAll(_children); }

bool ListItem::isRoot() const { return _id == 0; }

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
            success = child->setRowDb(i);
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

int ListItem::id() const { return _id; }

int ListItem::weight() const
{
    if (_isCancelled)
        return 2;
    if (_isCompleted)
        return 1;
    return 0;
}

QString ListItem::html() const { return _html; }
QString ListItem::text() const { return _text; }
QString ListItem::label() const { return _label; }

Qt::ItemFlags ListItem::flags() const { return _flags; };
void ListItem::setFlag(Qt::ItemFlags flag, bool state)
{
    if (state)
        _flags |= flag;
    else
        _flags &= ~flag;
}

int ListItem::childCount() const { return _children.count(); };

ListItem* ListItem::child(int row) const
{
    if (!(row >= 0 && row < _children.length()))
        return nullptr;
    return _children.at(row);
}

void ListItem::appendChild(ListItem* child)
{
    child->setParent(this, _children.length());
    _children.append(child);
}

void ListItem::insertChild(int row, ListItem* child)
{
    for (int i = row, n = _children.length(); i < n; ++i)
        _children.at(i)->adjustRow(1);
    child->setParent(this, row);
    _children.insert(row, child);
}

void ListItem::removeChild(int row)
{
    ListItem* item = _children.takeAt(row);
    if (item)
        delete item;
}

void ListItem::moveChild(int row) { _children.move(row, row + 1); }

bool ListItem::takeChildDb(int row)
{
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET weight = weight - 1 WHERE list_id = :list AND parent_id = :parent AND weight > :row");
    sql.bindValue(":list", _listId);
    sql.bindValue(":parent", _id);
    sql.bindValue(":row", row);
    return sql.exec();
}

ListItem* ListItem::takeChild(int row)
{
    for (int i = row + 1, n = _children.length(); i < n; ++i)
        _children.at(i)->adjustRow(-1);
    ListItem* child = _children.takeAt(row);
    child->setParent(nullptr, 0);
    return child;
}

int ListItem::row() const { return _row; };

void ListItem::setRow(int row) { _row = row; }

bool ListItem::setRowDb(int row)
{
    if (_row == row)
        return true;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET weight = :weight WHERE id = :id");
    sql.bindValue(":id", _id);
    sql.bindValue(":weight", row);
    if (sql.exec()) {
        setRow(row);
        return true;
    }
    return false;
}

void ListItem::adjustRow(int delta) { _row += delta; }

QString ListItem::markdown() const { return _markdown; };

void ListItem::setMarkdown(const QString& value)
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

ListItem* ListItem::parent() const { return _parent; };

void ListItem::setParent(ListItem* parent, int row)
{
    _parent = parent;
    if (parent)
        setLevel(parent->level() + 1);
    setRow(row);
}

bool ListItem::setParentDbOnly(ListItem* parent, int row)
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

int ListItem::level() const { return _level; };

void ListItem::setLevel(int level)
{
    _level = level;
    for (ListItem* child : _children)
        child->setLevel(level + 1);
}

bool ListItem::isExpanded() const { return _isExpanded; }

void ListItem::setExpanded(bool isExpanded) { _isExpanded = isExpanded; }

bool ListItem::setExpandedDb(bool isExpanded)
{
    if (isExpanded == _isExpanded)
        return true;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET is_expanded = :expanded WHERE id = :id");
    sql.bindValue("expanded", isExpanded ? 1 : 0);
    sql.bindValue("id", _id);
    if (sql.exec()) {
        setExpanded(isExpanded);
        return true;
    }
    return false;
}

bool ListItem::isCheckable() const { return _isCheckable; };
void ListItem::setCheckable(const bool isCheckable)
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
bool ListItem::setCheckableDb(const bool isCheckable)
{
    if (isCheckable == _isCheckable)
        return true;

    QSqlDatabase db = QSqlDatabase::database();
    bool localTrans = db.transaction();
    bool success = true;

    SqlQuery sql;
    sql.prepare("UPDATE list_item SET is_checkable = :checkable WHERE id = :id");
    sql.bindValue(":checkable", isCheckable ? 1 : 0);
    sql.bindValue(":id", _id);
    if (sql.exec()) {
        setCheckable(isCheckable);
        if (!isCheckable) {
            if (isCompleted())
                success = setCompletedDb(false);
            if (success)
                if (isCancelled())
                    success = setCancelledDb(false);
        }
    } else
        success = false;

    if (localTrans)
        if (success)
            db.commit();
        else
            db.rollback();

    return success;
}

bool ListItem::isCompleted() const { return _isCompleted; }
void ListItem::setCompleted(const bool isCompleted) { _isCompleted = isCompleted; }
bool ListItem::setCompletedDb(const bool isCompleted)
{
    if (isCompleted == _isCompleted)
        return true;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET is_completed = :completed WHERE id = :id");
    sql.bindValue(":completed", isCompleted);
    sql.bindValue(":id", _id);
    if (sql.exec()) {
        setCompleted(isCompleted);
        return true;
    }
    return false;
}

bool ListItem::isCancelled() const { return _isCancelled; }
void ListItem::setCancelled(const bool isCancelled) { _isCancelled = isCancelled; }
bool ListItem::setCancelledDb(const bool isCancelled)
{
    if (isCancelled == _isCancelled)
        return true;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET is_cancelled = :cancelled WHERE id = :id");
    sql.bindValue(":cancelled", isCancelled);
    sql.bindValue(":id", _id);
    if (sql.exec()) {
        setCancelled(isCancelled);
        return true;
    }
    return false;
}

bool ListItem::isProject() const { return _isProject; }
void ListItem::setProject(const bool isProject) { _isProject = isProject; }
bool ListItem::setProjectDb(const bool isProject)
{
    if (isProject == _isProject)
        return true;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET is_project = :project WHERE id = :id");
    sql.bindValue(":project", isProject);
    sql.bindValue(":id", _id);
    if (sql.exec()) {
        setProject(isProject);
        return true;
    }
    return false;
}

bool ListItem::isHighlighted() const { return _isHighlighted; }
void ListItem::setHighlighted(const bool isHighlighted) { _isHighlighted = isHighlighted; }
bool ListItem::setHighlightedDb(const bool isHighlighted)
{
    if (isHighlighted == _isHighlighted)
        return true;
    SqlQuery sql;
    sql.prepare("UPDATE list_item SET is_highlighted = :highlighted WHERE id = :id");
    sql.bindValue(":highlighted", isHighlighted ? 1 : 0);
    sql.bindValue(":id", _id);
    if (sql.exec()) {
        _isHighlighted = isHighlighted;
        return true;
    }
    return false;
}
