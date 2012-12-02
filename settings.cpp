#include "settings.h"
#include "main.h"
#include "math.h"
#include "messages.h"
#include "fileaccess.h"
#include <QStringList>
#include <QCoreApplication>

#include <qapplication.h>
#include <qsettings.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qwidget.h>
#include <QSettings>

QHash<QString, settingsItem> settings;

//
// process settings file
// or generate a default instance
//
QString Settings::getSetting(QString category, QString item, QString defaultValue)
{
  if (settings[category].contains(item))
    return settings[category][item];
  else
    return defaultValue;
}

//
// get RGB components out of an RGB string
//
void Settings::getRGBFromString(QString rgbString, QList<int>& rgbValues)
{
  QRegExp rx("(\\d+)[^\\d]+(\\d+)[^\\d]+(\\d+)[^\\d]+(\\d+)");
  rgbValues.clear();
  rgbValues << 255 << 255 << 255 << 255;
  rx.indexIn(rgbString);
  if (rx.captureCount() > 3)
  {
    for (int i=0; i<4; i++) rgbValues[i] = rx.cap(i+1).toInt();
  }
}

//
// set globals according to settings file
//
void Settings::setSettings(void)
{
  QList<int> rgb;
  QString rgbString;

  tableRowHeight   = Math::limit(getSetting("size", "rowHeight", "20").toInt(), 16, 30);
  tableColumnWidth = Math::limit(getSetting("size", "columnWidth", "80").toInt(), 40, 260);
  tableHeaderWidth = Math::limit(getSetting("size", "headerWidth", "180").toInt(), 40, 260);
  checkRemote      = Math::limit(getSetting("timer", "checkRemote", "1000").toInt(), 100, 60000);
  autoRefresh      = Math::limit(getSetting("timer", "autoRefresh", "60000").toInt(), 30000, 600000)/1000;

  mail->adminMail = getSetting("contact", "mail", "recipient@mail.dom");
  cursorActiveColorString = getSetting("color", "activeCursor", "rgb(160,160,255,255)");
  getRGBFromString(cursorActiveColorString, rgb);
  cursorActiveColor = QColor(rgb[0], rgb[1], rgb[2], rgb[3]);
  cursorInactiveColorString = getSetting("color", "inactiveCursor", "rgb(180,180,180,255)");
  getRGBFromString(cursorInactiveColorString, rgb);
  cursorInactiveColor = QColor(rgb[0], rgb[1], rgb[2], rgb[3]);
  rgbString = getSetting("color", "archiveBackground", "rgb(255,255,187,255)");
  getRGBFromString(rgbString, rgb);
  archiveBackgroundColor = QColor(rgb[0], rgb[1], rgb[2], rgb[3]);
  rgbString = getSetting("color", "weekEndBackground", "rgb(220,220,220,255)");
  getRGBFromString(rgbString, rgb);
  weekEndColor = QColor(rgb[0], rgb[1], rgb[2], rgb[3]);
}

//
// process settings file
// or generate a default instance
//
bool Settings::getSettingsFile(QString settingfile)
{
  bool ok = false;
  if (QFile(settingfile).exists())
  {
    FileAccess file(settingfile);
    if (file.read())
    {
      QByteArray ba;
      file.get(ba);
      QString text = QVariant(ba).toString();
      //text.replace(QRegExp("#[^\\r\\n]+"), ""); // remove comments
      text.replace('\r', "");
      QStringList lines = text.split("\n", QString::SkipEmptyParts);
      QString trailer;
      for (int i=0; i<lines.count(); i++)
      {
        if ((lines.at(i).contains(':')) && (lines.at(i).contains('=')))
        {
          QString line = lines.at(i);
          line.replace(QRegExp("^\\s+"),"");
          line.replace(QRegExp("\\s+$"),"");
          QStringList categoryKeyValue = line.split(QRegExp("(\\s*:\\s*|\\s*=\\s*)"), QString::SkipEmptyParts);
          if (categoryKeyValue.count() == 3)
          {
            if (!settings[categoryKeyValue[0]].contains(categoryKeyValue[1]))
            {
              settings[categoryKeyValue[0]][categoryKeyValue[1]] = categoryKeyValue[2];
              Msg::log(MSG_NOTE, QString("beállítás %1(%2)=%3").arg(categoryKeyValue[0]).arg(categoryKeyValue[1]).arg(categoryKeyValue[2]));
            }
            if (!trailer.isEmpty())
            {
              settings[categoryKeyValue[0]]["header"] = trailer;
              trailer.clear();
            }
          }
        }
        else
        {
          trailer = QString("%1\r\n%2").arg(trailer).arg(lines.at(i));
        }
      }
      ok = true;
    }
  }
  return ok;
}

//
// write settings file
//
void Settings::putSettingsFile(void)
{
  QString settingsText("");
  QHashIterator<QString, settingsItem> sections(settings);
  while (sections.hasNext())
  {
    sections.next();
    QString sectionName(sections.key());
    QString header("");
    QString body("");
    QHashIterator<QString, QString> setting(sections.value());
    while (setting.hasNext())
    {
      setting.next();
      if (setting.key() == "header")
      {
        header = QString(setting.value());
      }
      else
      {
        body = QString("%1\r\n%2 = %3").arg(body).arg(QString("%1:%2                                                  ")
                                       .arg(sectionName).arg(setting.key()).left(40))
                                       .arg(setting.value());
      }
    }
    settingsText = QString("%1%2%3\r\n").arg(settingsText).arg(header).arg(body).remove(QRegExp("^\\s+"));
  }
  FileAccess file(pth.pathes[FLE_settings]);
  QByteArray ba = QVariant(settingsText).toByteArray();
  file.put(ba);
  file.write();
}
