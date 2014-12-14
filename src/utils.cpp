#include "utils.h"

#include "debug.h"
#include "settings.h"

#include <QDir>
#include <QFile>
#include <QDate>
#include <QFontDatabase>

namespace Util
{
    static QHash<QString, QIcon> iconsCache;

    static QIcon _findIcon(const QString& file)
    {
        if (runtimeSettings.contains("theme")) {
            QDir iconsDir(runtimeSettings["theme"].toString());
            if (iconsDir.exists() && iconsDir.exists(file)) {
                return QIcon(iconsDir.filePath(file));
            }
        }
        return QIcon(":" + file);
    }

    QIcon findIcon(const QString& name)
    {
        if (iconsCache.contains(name))
            return iconsCache[name];
        QIcon icon = _findIcon(name + ".png");
        iconsCache[name] = icon;
        return icon;
    }

    QIcon findIcon(const QString& name, QList<int> altSizes)
    {
        QIcon icon = _findIcon(name + ".png");
        for (int size : altSizes) {
            QString altFile = findIconFile(name + QString::number(size));
            if (!altFile.isNull())
                icon.addFile(altFile, QSize(size, size));
        }
        return icon;
    }

    QString findIconFile(const QString& name)
    {
        QString file = name + ".png";
        if (runtimeSettings.contains("theme")) {
            QDir iconsDir(runtimeSettings["theme"].toString());
            if (iconsDir.exists() && iconsDir.exists(file))
                return iconsDir.filePath(file);
        }
        return file.prepend(":");
    }

    void loadCustomFonts()
    {
        QDir fontsDir("fonts");
        if (fontsDir.exists())
            for (QString fontFile : fontsDir.entryList(QDir::Files))
                if (fontFile.endsWith(".ttf") || fontFile.endsWith(".otf"))
                    QFontDatabase::addApplicationFont(fontsDir.filePath(fontFile));
    }

    QDate parseDate(const QString& text)
    {
        if (text == "today")
            return QDate::currentDate();
        if (text == "tomorrow")
            return QDate::currentDate().addDays(1);
        return QDate();
    }
}
