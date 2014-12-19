#include "constants.h"

static QColor _color(const QString& hex, int alpha = 255)
{
    QColor c;
    c.setNamedColor(hex);
    c.setAlpha(alpha);
    return c;
}

namespace App
{
    const QColor HighlightBackgroundColor = _color("#F1C40F", 128);
    const QColor ProjectBackgroundColor = _color("#3498DB", 32);
    const QColor MilestoneBackgroundColor = _color("#2ECC71", 32);

    const QColor CancelledColor = QColor(182, 124, 124);
    const QColor CompletedColor = QColor(Qt::gray);
    const QColor NoteColor = _color("#666666");

    const QColor Priority1Color = _color("#C0392B");
    const QColor Priority2Color = _color("#E67E22");
    const QColor Priority3Color = _color("#27AE60");
}
