#pragma once

#include <QString>
#include <QVariant>

class Settings
{
public:
    static void load();
    int getInt(const QString& key);
    QString getString(const QString& key);
private:
    QHash<QString, QVariant> settings;
};
