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
    const QColor HighlightBackgroundColor = _color("#F39C12", 128);
    const QColor ProjectBackgroundColor = _color("#3498DB", 32);
    const QColor MilestoneBackgroundColor = _color("#2ECC71", 32);
}
