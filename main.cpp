#include <QtGui/QApplication>
#include "main.h"
#include "mainwindow.h"
#include "splashscreen.h"
#include "settings.h"
#include "messages.h"
#include <windows.h>
#include <stdio.h>
#include <lmcons.h>
#include <QTextCodec>
#include <QDebug>
#include <QLocale>
#include <QDir>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDirIterator>

QString         correctAdminKey = "AE3FF9435120484DB4BA03C17E02FD8E";
QSettings       appSettings("CashFlow", "ApplicationSettings");
Pathes          pth;
bool            adminAccess = false;
SplashScreen*   splash = NULL;
QDesktopWidget* desktop = NULL;
QTextBrowser*   logBox = NULL;
QString         userName;
QString         installedVersion;
QString         adminKey;

QFont           tableHeaderFont("Tahoma",   8);
QFont           logWindowFont(  "Courier",  8);
QFont           tableCellFont(  "Tahoma",   8);
QFont           textEditFont(   "Courier",  8);
QFont           calendarFont(   "Tahoma",   8);
QFont           comboBoxFont(   "Tahoma",  12, QFont::Bold);

QColor          cursorActiveColor;
QString         cursorActiveColorString;
QColor          cursorInactiveColor;
QString         cursorInactiveColorString;
QColor          weekEndColor;
QColor          archiveBackgroundColor;

bool            remote = true; // remote database is accessible
bool            autoUpdateFlag;
bool            outputLogWarningsFlag;
bool            outputLogInfosFlag = true;
bool            outputLogNotesFlag;
bool            summaryVisible;
bool            fileLoggingSuppressed = false;

int             tableRowHeight;
int             tableColumnWidth;
int             sumColumnWidth;
int             tableHeaderWidth;
int             checkRemote;
int             autoRefresh;

TextEdit*       settingsWindow;
TextEdit*       logWindow;
TextEdit*       tableEditWindow;
Mail*           mail;

__declspec(dllimport) int qt_ntfs_permission_lookup;

//
// get Windows username
//
QString getUserName(void)
{
  LPTSTR lpszSystemInfo; // pointer to system information
  DWORD cchBuff = 256; // size of user name
  TCHAR tchBuffer[UNLEN + 1]; // buffer for expanded string
  lpszSystemInfo = tchBuffer;
  GetUserName(lpszSystemInfo, &cchBuff);
  uint i = 0;
  if (lpszSystemInfo == NULL)
  {
    userName = "DEFAULT_USER";
  }
  else
  {
    while (i < cchBuff*2)
    {
      userName.append(((char*)lpszSystemInfo + i));
      i+=2;
    }
  }
  return userName;
}

void checkNextVersion(void)
{
  installedVersion = appSettings.value("InstalledVersion", "1.0.0").toString();
  QStringList nameFilter;
  nameFilter << "cashflow_v*.exe";
  QDir databaseDir(pth.pathes[PTH_installer]);
  QStringList fileList = databaseDir.entryList(nameFilter, QDir::Files, QDir::Unsorted);
  QRegExp versionPattern("cashflow_v(\\d\\.\\d\\.\\d)\\.exe");
  QList<QString> versionList = installedVersion.split(".");
  for (int f=0; f<fileList.count(); f++)
  {
    versionPattern.indexIn(fileList[f]);
    if ((versionPattern.captureCount() > 0) &&
        (!versionPattern.cap(1).isEmpty()))
    {
      QList<QString> foundList = versionPattern.cap(1).split(".");
      if ( (foundList[0].toInt() > versionList[0].toInt()) ||
          ((foundList[0].toInt() == versionList[0].toInt()) &&
           (foundList[1].toInt() >  versionList[1].toInt())) ||
          ((foundList[0].toInt() == versionList[0].toInt()) &&
           (foundList[1].toInt() == versionList[1].toInt()) &&
           (foundList[2].toInt() >  versionList[2].toInt())))
        Msg::show(MSG_INFO, MSG_NEWVERSION, fileList[f]);
    }
  }
}

//
// remove obsolete settings
//
void removeSettings(void)
{
}

//
// merge settings files
//
void mergeSettings(void)
{
  QString newSettingsFile = pth.pathes[FLE_settings];
  newSettingsFile.replace(".txt","_temp.txt");
  Settings::getSettingsFile(pth.pathes[FLE_settings]);
  Settings::getSettingsFile(newSettingsFile);
  removeSettings();
  Settings::putSettingsFile();
  Settings::setSettings();
  QFile(newSettingsFile).remove();
}

//
// service for directory file list
//
QStringList getFileList(QString folder, QString suffix, bool recursive)
{
  QStringList fileList;
  QString path(QDir(folder).absolutePath());
  QDirIterator dirIt(path, QDirIterator::Subdirectories);
  while (dirIt.hasNext())
  {
    dirIt.next();
    if ((recursive) || (dirIt.fileInfo().absolutePath() == path))
    {
      QString file(dirIt.fileInfo().absoluteFilePath());
      if (QFileInfo(dirIt.filePath()).isFile())
        if (QFileInfo(dirIt.filePath()).suffix() == suffix)
          fileList.append(dirIt.filePath());
    }
  }
  qSort(fileList);
  return fileList;
}

//
// main caller
//
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  qt_ntfs_permission_lookup = 0;
  userName = getUserName();
  desktop = QApplication::desktop();
  QLocale::setDefault(QLocale(QLocale::Hungarian));
  QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1250"));
  checkNextVersion();
  if (pth.checkPathes())
  {
    mail = new Mail(&app);
    mergeSettings();
    splash = new SplashScreen(desktop);
    splash->setUpdatesEnabled(false);
    splash->container = desktop;
    logBox = new QTextBrowser(NULL);
    splash->message(QString("<b><font color=black>CashFlow %1</font><br></b>").arg(installedVersion), true);
    splash->setUpdatesEnabled(true);
    MainWindow w;
    splash->container = &w;
    w.show();
    splash->finish();
    return app.exec();
  }
  else
  {
    QMessageBox::information(NULL, "CashFlow", "Az alkalmazás valamelyik útvonala érvénytelen.");
    return 1;
  }
}

