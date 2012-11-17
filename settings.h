#ifndef SETTINGS_H
#define SETTINGS_H

#include "main.h"
#include <QString>
#include <QTextEdit>

typedef QHash<QString, QString> settingsItem;
extern QHash<QString, settingsItem> settings;

class Settings
{
public:
  Settings();
  static bool         getSettingsFile(QString settingfile);
  static void         putSettingsFile(void);
  static QString      getSetting(QString category, QString item, QString defaultValue);
  static void         setSettings(void);

private:
  static void         getRGBFromString(QString rgbString, QList<int>& rgbValues);
};

#endif // SETTINGS_H
