#pragma once

#include <QColor>

#define SQL_NULL QVariant()

namespace App
{
enum Direction { Up, Down, Left, Right };
enum ItemDataRole { ExpandedStateRole = Qt::UserRole + 10, OriginalTextRole, ProjectRole };
enum EditorTab { SimpleEditorTab = 0, TextEditorTab };
enum SortMode { SortAll, SortCompleted };
enum AppendMode { AppendChild, AppendBefore, AppendAfter };

const QColor ProjectBackgroundColor = QColor::fromHsv(200, 80, 255);
}
