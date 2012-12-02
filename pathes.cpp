#include "pathes.h"
#include "messages.h"
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QDate>

static const int criticalPathCount = 4;
static int critical[criticalPathCount] =
{
  (int)PTH_working,
  (int)PTH_localDatabase,
  (int)PTH_localArchiveDatabase,
  (int)PTH_importExport
};

static QString symbols[FLE_count] =
{
  "TELEPÍTÕ:_______________",
  "FUTÓ ALKALMAZÁS:________",
  "TELEPÍTETT_ALKALMAZÁS:__",
  "KÖZPONTI_ADATBÁZIS:_____",
  "ARCHÍV ADATBÁZIS:_______",
  "MUNKAKÖNYVTÁR:__________",
  "HELYI_ADABÁZIS:_________",
  "HELYI ARCHÍV ADATBÁZIS:_",
  "IMPORT ÉS EXPORT:_______",
  "SEGÍTSÉG:_______________",
  "BEÁLLÍTÁSOK:____________",
  "NAPLÓ:__________________"
};

Pathes::Pathes()
{
  pathesAreOk = true;
  QSettings appSettings("CashFlow", "ApplicationSettings");
  for (int p=0; p<FLE_count; p++) pathes.append("");

  pathes[PTH_application]          = QDir::currentPath();
  pathes[PTH_installedApplication] = appSettings.value("ApplicationPath", "C:/Program Files/CashFlow").toString().replace("\\", "/");
  pathes[FLE_help]                 = QString("%1/help/index.htm").arg(pathes[PTH_installedApplication]);

  pathes[PTH_remoteDatabase]       = appSettings.value("DatabasePath", "L:/CashFlow/database").toString().replace("\\", "/");
  pathes[PTH_installer]            = pathes[PTH_remoteDatabase];
  pathes[PTH_archiveDatabase]      = QString("%1/archive").arg(pathes[PTH_remoteDatabase]);

  pathes[PTH_working]              = appSettings.value("WorkingPath", "C:/CashFlow").toString().replace("\\", "/");
  pathes[FLE_settings]             = QString("%1/settings.txt").arg(pathes[PTH_working]);
  pathes[FLE_log]                  = QString("%1/log.txt").arg(pathes[PTH_working]);
  pathes[PTH_localDatabase]        = QString("%1/database").arg(pathes[PTH_working]);
  pathes[PTH_localArchiveDatabase] = QString("%1/archive").arg(pathes[PTH_localDatabase]);
  pathes[PTH_importExport]         = QString("%1/export").arg(pathes[PTH_working]);

  pathes[PTH_installer].remove(QRegExp("/[^/]+$"));
  createUserPathes();
}

void Pathes::createUserPathes(void)
{
  for (int p=0; p<criticalPathCount; p++)
    createPath(pathes[critical[p]]);
}

void Pathes::createPath(QString path)
{
  if (!QDir(path).exists())
  {
    QDir dir;
    if (dir.mkdir(path))
    {
      Msg::log(MSG_NOTE, QString("Az útvonal létrehozva: %1").arg(path));
    }
    else
    {
      QMessageBox::information(NULL, "CashFlow", QString("Az útvonal nem hozható létre: %1").arg(path));
      Msg::log(MSG_ERROR, QString("Az útvonal nem hozható létre: %1").arg(path));
      pathesAreOk = false;
    }
  }
}

bool Pathes::checkPathes(void)
{
  if ((pathesAreOk) ||
      (pathes[PTH_application] == pathes[PTH_installedApplication]))
    return true;
  else
    return false;
}

QString Pathes::getLog(void)
{
  QString output("");
  for (int p=0; p<FLE_count; p++) output.append(QString("%1 %2<br>\r\n").arg(symbols[p]).arg(pathes[p]));
  return output;
}

