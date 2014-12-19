#pragma once

#include "constants.h"
#include "markdownrenderer.h"

#include <QModelIndex>
#include <QDateTime>

class ListModel;

class ListItem : public QObject
{
    Q_OBJECT
public:
    // root
    ListItem(ListModel* model, int listId);
    // new item
    ListItem(int listId, int id, QString content);
    // existing item
    ListItem(int listId, int id, QString content, bool isExpanded, bool isProject, bool isMilestone, bool isHighlighted,
             bool isCheckable, bool isCompleted, bool isCancelled, QDate dueDate, int priority);
    ~ListItem();

    ListModel* model() const { return _model; };
    void setModel(ListModel* model);

    bool isRoot() const { return _isRoot; };
    bool sort(App::SortMode mode);

    int id() const { return _id; };
    int weight() const;

    QString html() const;
    QString text() const { return _text; };
    QString label() const { return _label; };

    Qt::ItemFlags flags() const { return _flags; };
    void setFlag(Qt::ItemFlags flag, bool state = true);

    int childCount() const { return _children.length(); };
    ListItem* child(int row) const;
    ListItem* firstChild() const;
    ListItem* lastChild() const;
    void appendChild(ListItem* child);
    void insertChild(int row, ListItem* child);
    void removeChild(int row);
    void moveChild(int row);
    bool takeChildDb(int row);
    ListItem* takeChild(int row);
    bool isLastChild() const { return _parent && _parent->childCount() == _row + 1; };
    bool isNote() const { return !_isCheckable && !_isProject && !_isMilestone && !_isHighlighted && _children.length() == 0; };

    int row() const { return _row; };
    bool setRow(int row);

    QString markdown() const { return _markdown; };
    bool setMarkdown(const QString& value);

    ListItem* parent() const { return _parent; };
    void setParent(ListItem* parent, int row);
    bool setParentDb(ListItem* parent, int row);

    int level() const { return _level; };
    void setLevel(const int level);

    bool isExpanded() const { return _isExpanded; };
    bool setExpanded(bool isExpanded);

    bool isCheckable() const { return _isCheckable; };
    bool setCheckable(const bool isCheckable);

    bool isCompleted() const { return _isCompleted; };
    bool setCompleted(const bool isCompleted);

    bool isCancelled() const { return _isCancelled; };
    bool setCancelled(const bool isCancelled);

    bool isProject() const { return _isProject; };
    bool setProject(const bool isProject);

    bool isMilestone() const { return _isMilestone; };
    bool setMilestone(const bool isMilestone);

    bool isHighlighted() const { return _isHighlighted; };
    bool setHighlighted(const bool isHighlighted);

    QDate dueDate() const { return _dueDate; };
    bool setDueDate(const QDate& dueDate);

    int priority() const { return _priority; };
    bool setPriority(int priority);
signals:
    void scheduleChanged();
    void operationError(const QString& message);
private:
    ListModel* _model{nullptr};

    bool _isRoot{false};
    int _listId{0};
    int _id{0}; // 0 means root
    int _row{0};
    int _priority{0};

    QString _markdown;
    QString _html;
    QString _text;
    QString _label;

    bool _isProject{false};
    bool _isMilestone{false};
    bool _isExpanded{false};
    bool _isCheckable{false};
    bool _isCompleted{false};
    bool _isCancelled{false};
    bool _isHighlighted{false};
    QDate _dueDate;

    ListItem* _parent{nullptr};
    int _level{0};
    Qt::ItemFlags _flags;

    QList<ListItem*> _children;

    static const MarkdownRenderer renderer;

    void _adjustRow(int delta) { _row += delta; };
    void _setMarkdown(const QString& value);
    void _setCheckable(bool isCheckable);
    bool _setAttribute(const QString& column, QVariant value) const;
};
