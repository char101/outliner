#pragma once

#include <QColor>

#define SQL_NULL QVariant()

namespace App
{
enum Direction { Up, Down, Left, Right };
enum ItemDataRole { ExpandedStateRole = Qt::UserRole + 10, OriginalTextRole, ProjectRole };
enum EditorTab { SimpleEditorTab = 0, TextEditorTab };
enum SortMode { SortByStatus, SortByStatusAndContent };
enum AppendMode { AppendChild, AppendBefore, AppendAfter };
enum ItemState { CheckableState, CompletedState, CancelledState, ProjectState, HighlightedState };

extern const QColor HighlightBackgroundColor;
extern const QColor ProjectBackgroundColor;
extern const QColor MilestoneBackgroundColor;
}
