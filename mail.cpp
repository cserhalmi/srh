#include "mail.h"
#include <QSettings>
#include <QStringList>
#include <QProcess>
#include <QApplication>

Mail::Mail(QApplication* app)
{
  application = app;
  QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE", QSettings::NativeFormat);
  outlook = settings.value("Path", "").toString();
  if (!outlook.isEmpty())
    outlook.append("outlook.exe");
}

void Mail::provideMailToSend(QString attachmentPath)
{
  if (!outlook.isEmpty())
  {
    QStringList arguments;
    arguments << "/m" << adminMail << "/a" << attachmentPath;
    QProcess* process = new QProcess();
    process->startDetached(outlook, arguments);
  }
}
