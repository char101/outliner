#include "utils.h"

#include <QDir>

QIcon Util::findIcon(const QString& icon)
{
    QDir icons("icons");
    QString file = icon;
    if (!file.endsWith(".png"))
        file.append(".png");
    return QIcon(icons.exists(file) ? icons.filePath(file) : QString(file).prepend(":"));
}
