#include <QIcon>
#include <QString>

namespace Util
{
    QIcon findIcon(const QString& name);
    QIcon findIcon(const QString& name, QList<int> altSizes);
    QString findIconFile(const QString& name);
    QDate parseDate(const QString& text);
    void loadCustomFonts();
}
