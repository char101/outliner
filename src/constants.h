#pragma once

#define SQL_NULL QVariant()

namespace App
{
enum Direction { Up, Down, Left, Right };
enum ItemDataRole { ExpandedStateRole = Qt::UserRole + 10, OriginalTextRole };
enum EditorTab { SimpleEditorTab = 0, TextEditorTab };
}
