#pragma once

#include "markdownrenderer.h"
#include "listmodel.h"

#include <QModelIndex>

class ListItem
{
public:
    // root
    ListItem(int listId);
    // new item
    ListItem(int listId, int id, QString content);
    // existing item
    ListItem(int listId, int id, QString content, bool isExpanded, bool isProject, bool isHighlighted,
             bool isCheckable, bool isCompleted, bool isCancelled);
    ~ListItem();

    bool isRoot() const;
    bool sort(App::SortMode mode);

    int id() const;
    int weight() const;

    QString html() const;
    QString text() const;
    QString label() const;

    Qt::ItemFlags flags() const;
    void setFlag(Qt::ItemFlags flag, bool state = true);

    int childCount() const;
    ListItem* child(int row) const;
    void appendChild(ListItem* child);
    void insertChild(int row, ListItem* child);
    void removeChild(int row);
    void moveChild(int row);
    bool takeChildDb(int row);
    ListItem* takeChild(int row);

    int row() const;
    void setRow(int value);
    bool setRowDb(int value);
    void adjustRow(int delta);

    QString markdown() const;
    void setMarkdown(const QString& value);

    ListItem* parent() const;
    void setParent(ListItem* parent, int row);
    bool setParentDbOnly(ListItem* parent, int row);

    int level() const;
    void setLevel(const int level);

    bool isExpanded() const;
    void setExpanded(bool state);
    bool setExpandedDb(bool state);

    bool isCheckable() const;
    void setCheckable(const bool isCheckable);
    bool setCheckableDb(const bool isCheckable);

    bool isCompleted() const;
    void setCompleted(const bool isCompleted);
    bool setCompletedDb(const bool isCompleted);

    bool isCancelled() const;
    void setCancelled(const bool isCancelled);
    bool setCancelledDb(const bool isCancelled);

    bool isProject() const;
    void setProject(const bool isProject);
    bool setProjectDb(const bool isProject);

    bool isHighlighted() const;
    void setHighlighted(const bool isHighlighted);
    bool setHighlightedDb(const bool isHighlighted);
private:
    int _listId{0};
    int _id{0};
    int _row{0};

    QString _markdown;
    QString _html;
    QString _text;
    QString _label;

    bool _isProject{false};
    bool _isExpanded{false};
    bool _isCheckable{false};
    bool _isCompleted{false};
    bool _isCancelled{false};
    bool _isHighlighted{false};

    ListItem* _parent{nullptr};
    int _level{0};
    Qt::ItemFlags _flags;

    QList<ListItem*> _children;

    static const MarkdownRenderer renderer;
};
