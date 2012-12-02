#include "debug.h"
#include "main.h"
#include "messages.h"
#include "settings.h"
#include <QIcon>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QScrollBar>
#include <QString>

static void setMsgBox(QMessageBox& msgBox)
{
  QSpacerItem* horizontalSpacer = new QSpacerItem(400, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  QGridLayout* layout = (QGridLayout*)msgBox.layout();
  layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
  msgBox.setWindowTitle("CashFlow");
  msgBox.setWindowIcon(ICON("logo.png"));
  msgBox.setMinimumSize(800,600);
  msgBox.setWindowFlags(Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
  msgBox.setWindowModality(Qt::ApplicationModal);
  msgBox.setFocusPolicy(Qt::StrongFocus);
}

Msg::Msg(QWidget* parent)
{
  Q_UNUSED(parent);
  setObjectName("Messages");
  D_CONSTRUCT("")
}

void Msg::saveStartupStateToLogFile()
{
  QFile file(pth.pathes[FLE_log]);
  if (file.open(QIODevice::WriteOnly))
  {
    QString outtext;
    outtext.append(tr("INDÍTÁS:________________ %1<br>\r\n").arg(QDateTime::currentDateTime().toString("yyyy/MMM/dd hh:mm:ss")));
    outtext.append(tr("FELHASZNÁLÓ:____________ %1<br>\r\n").arg(userName));
    outtext.append(tr("KAPCSOLAT:______________ %1<br>\r\n").arg(remote ? "központi" : "helyi"));
    outtext.append(tr("HOZZÁFÉRÉS:_____________ %1<br>\r\n").arg(adminKey == correctAdminKey ? "adminisztrátor" : "felhasználó"));
    outtext.append(pth.getLog());
    file.write(QVariant(outtext).toByteArray());
    file.close();
  }
  else
  {
    Msg::log(MSG_ERROR, tr("a %1 napló fájl írása sikertelen").arg(pth.pathes[FLE_log]));
  }
}

bool Msg::clearLog()
{
  logBox->clear();
  if (QFile(pth.pathes[FLE_log]).remove())
  {
    Msg::saveStartupStateToLogFile();
    return true;
  }
  else
  {
    Msg::saveStartupStateToLogFile();
    return false;
  }
}

void Msg::log(msgLevel type, QString text)
{
  QString typeColor;
  switch(type)
  {
  case MSG_ERROR:
  typeColor.append("red");
  break;
  case MSG_WARNING:
  typeColor.append("blue");
  break;
  case MSG_INFO:
  typeColor.append("green");
  break;
  case MSG_NOTE:
  typeColor.append("gray");
  break;
  default:
    typeColor = "black";
  }
  if (logBox != NULL)
  {
    text.replace(pth.pathes[PTH_localDatabase], "helyi ");
    text.replace(pth.pathes[PTH_remoteDatabase], "központi ");
    QString outtext = QString("<font color=\"%1\">[%2]</font> %3<br>").arg(typeColor).arg(QDateTime::currentDateTime().toString("yyyy/MMM/dd hh:mm:ss")).arg(text);
    QFile file(pth.pathes[FLE_log]);
    if (!fileLoggingSuppressed)
    {
      if (file.open(QIODevice::Append))
      {
        file.write(QVariant(outtext).toByteArray());
        file.write("\r\n");
        file.close();
      }
      else
      {
        Msg::log(MSG_ERROR, tr("a %1 napló fájl írása sikertelen").arg(pth.pathes[FLE_log]));
      }
    }
    if ((outputLogWarningsFlag && (type == MSG_WARNING)) ||
        (outputLogInfosFlag && (type == MSG_INFO)) ||
        (outputLogNotesFlag && (type == MSG_NOTE)) ||
        (type == MSG_ERROR))
    {
      logBox->moveCursor(QTextCursor::Start);
      logBox->insertHtml(outtext);
      if ((logBox->isVisible()) && (logBox->updatesEnabled()))
      {
        logBox->hide();
        logBox->show();
      }
    }
  }
}

int Msg::error(msgType message, QStringList *textList)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  setMsgBox(msgBox);
  switch(message)
  {
  case MSG_FILE_SAVE:
  {
    msgBox.setText(tr("A fájl mentése sikertelen:"));
    msgBox.setInformativeText(tr("%1\r\nEllen\365rizze az útvonalat, illetve hogy a fájlt nincs megnyitva más alkalmazásban.").arg(textList->at(0)));
  }
  break;
  default:
    break;
  }
  int answer = msgBox.exec();
  QCoreApplication::exit(1);
  QApplication::exit(1);
  return answer;
}

int Msg::warning(msgType message, QStringList *textList)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Warning);
  setMsgBox(msgBox);
  switch(message)
  {
  default:
    break;
  }
  return msgBox.exec();
}

int Msg::information(msgType message, QStringList *textList)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Information);
  setMsgBox(msgBox);
  switch(message)
  {
  case MSG_NEWVERSION:
  {
    msgBox.setText(tr("Új verzió elérhet\365: %1").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Ok);
  }
  break;
  default:
      break;
  }
  return msgBox.exec();
}

int Msg::question(msgType message, QStringList *textList)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Question);
  setMsgBox(msgBox);
  switch(message)
  {
  case MSG_FILE_SAVE:
  {
    msgBox.setText(tr("A fájl megválozott. Kívánja menteni?"));
    msgBox.setInformativeText(tr("%1").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
    msgBox.setDefaultButton(QMessageBox::Save);
  }
  break;
  case MSG_CLEARLOGGEDDATA:
  {
    msgBox.setText(tr("Törli a napló bejegyzéseit?"));
    msgBox.setInformativeText("Törlés hatására az alkalmazás könyvtárban vezetett log.txt fájl adatai elvesznek. Felhasználás hibakeresésre csak az ezután keletkezett események alapján lehetséges.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_CLEARLOCALDATAFILE:
  {
    msgBox.setText(tr("Törli a helyi adatbázis fájlt?"));
    msgBox.setInformativeText("Törlés hatására a legutolsó frissítés óta bevitt adatok elvesznek, de a központi adatbázisban tároltak nem. Frissítés után a legutolsó tárolt állapot jelenik meg.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_CLEARREMOTEDATAFILE:
  {
    msgBox.setText(tr("Törli a központi adatbázis fájlt?"));
    msgBox.setInformativeText(tr("%1 törlésének hatására az adatok elvesznek, de az archívumban tároltak nem. Adatbázis hiba esetén az archív példány az adatai fájl másolással visszaállíthatók.").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_CLEARREMOTEDATA:
  {
    msgBox.setText(tr("Törli a központi adatbázis adatait?"));
    msgBox.setInformativeText(tr("%1 kiürítésének hatására frissítéskor a felhasználók adatai is törl\365dnek.").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_ERRORINLOG:
  {
    msgBox.setText(tr("A napló fájl hiba bejegyzéseket tartalmaz."));
    msgBox.setInformativeText(tr("Elküldi az adminisztrátornak?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  default:
    break;
  }
  return msgBox.exec();
}

int Msg::pshow(msgLevel level, msgType message)
{
  QString text("");
  QStringList sl = QStringList(text);
  return show(level, message, &sl);
}

int Msg::show(msgLevel level, msgType message, QString text)
{
  if (!pth.pathes[PTH_localDatabase].isEmpty())
    text.replace(pth.pathes[PTH_localDatabase], "helyi ");
  if (!pth.pathes[PTH_remoteDatabase].isEmpty())
    text.replace(pth.pathes[PTH_remoteDatabase], "központi ");
  QStringList sl = QStringList(text);
  return show(level, message, &sl);
}

int Msg::show(msgLevel level, msgType message, QStringList* textList)
{
  switch(level)
  {
  case MSG_ERROR:    return error(message, textList);
    break;
  case MSG_WARNING:  return warning(message, textList);
    break;
  case MSG_INFO:     return information(message, textList);
    break;
  case MSG_QUESTION: return question(message, textList);
    break;
  default:
    break;
  }
  return 0;
}
