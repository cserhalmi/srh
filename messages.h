#ifndef MESSAGES_H
#define MESSAGES_H

#include <QMessageBox>
#include <QObject>
#include <QTextBrowser>

enum msgType
{
  MSG_FILE_SAVE,
  MSG_CLEARLOGGEDDATA,
  MSG_CLEARLOCALDATAFILE,
  MSG_CLEARREMOTEDATAFILE,
  MSG_CLEARREMOTEDATA,
  MSG_NEWVERSION
};
enum msgLevel
{
  MSG_ERROR,
  MSG_WARNING,
  MSG_INFO,
  MSG_NOTE,
  MSG_QUESTION
};

class Msg : QObject
{

  Q_OBJECT

public:
  explicit    Msg(QWidget *parent);
  static int  show(msgLevel level = MSG_INFO, msgType message = MSG_FILE_SAVE, QStringList *texList = 0);
  static int  show(msgLevel level = MSG_INFO, msgType message = MSG_FILE_SAVE, QString text = "");
  static int  pshow(msgLevel level = MSG_INFO, msgType message = MSG_FILE_SAVE);
  static void log(msgLevel type = MSG_INFO, QString text = "");
  static bool clearLog();
  static void saveStartupStateToLogFile();

private:
  static int error(msgType message = MSG_FILE_SAVE, QStringList *texList = 0);
  static int warning(msgType message = MSG_FILE_SAVE, QStringList *texList = 0);
  static int information(msgType message = MSG_FILE_SAVE, QStringList *texList = 0);
  static int question(msgType message = MSG_FILE_SAVE, QStringList *texList = 0);

signals:
  void errorMessageLaunched();

};

#endif // MESSAGES_H
