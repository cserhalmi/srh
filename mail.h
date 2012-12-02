#ifndef MAIL_H
#define MAIL_H

#include <QString>
#include <QApplication>

class Mail
{
public:
  Mail(QApplication*);
  void provideMailToSend(QString attachmentPath);
  QString adminMail;

private:
  QString outlook;
  QApplication* application;

};

#endif // MAIL_H
