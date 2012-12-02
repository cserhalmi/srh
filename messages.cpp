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
    outtext.append(tr("IND�T�S:________________ %1<br>\r\n").arg(QDateTime::currentDateTime().toString("yyyy/MMM/dd hh:mm:ss")));
    outtext.append(tr("FELHASZN�L�:____________ %1<br>\r\n").arg(userName));
    outtext.append(tr("KAPCSOLAT:______________ %1<br>\r\n").arg(remote ? "k�zponti" : "helyi"));
    outtext.append(tr("HOZZ�F�R�S:_____________ %1<br>\r\n").arg(adminKey == correctAdminKey ? "adminisztr�tor" : "felhaszn�l�"));
    outtext.append(pth.getLog());
    file.write(QVariant(outtext).toByteArray());
    file.close();
  }
  else
  {
    Msg::log(MSG_ERROR, tr("a %1 napl� f�jl �r�sa sikertelen").arg(pth.pathes[FLE_log]));
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
    text.replace(pth.pathes[PTH_remoteDatabase], "k�zponti ");
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
        Msg::log(MSG_ERROR, tr("a %1 napl� f�jl �r�sa sikertelen").arg(pth.pathes[FLE_log]));
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
    msgBox.setText(tr("A f�jl ment�se sikertelen:"));
    msgBox.setInformativeText(tr("%1\r\nEllen\365rizze az �tvonalat, illetve hogy a f�jlt nincs megnyitva m�s alkalmaz�sban.").arg(textList->at(0)));
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
    msgBox.setText(tr("�j verzi� el�rhet\365: %1").arg(textList->at(0)));
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
    msgBox.setText(tr("A f�jl megv�lozott. K�v�nja menteni?"));
    msgBox.setInformativeText(tr("%1").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
    msgBox.setDefaultButton(QMessageBox::Save);
  }
  break;
  case MSG_CLEARLOGGEDDATA:
  {
    msgBox.setText(tr("T�rli a napl� bejegyz�seit?"));
    msgBox.setInformativeText("T�rl�s hat�s�ra az alkalmaz�s k�nyvt�rban vezetett log.txt f�jl adatai elvesznek. Felhaszn�l�s hibakeres�sre csak az ezut�n keletkezett esem�nyek alapj�n lehets�ges.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_CLEARLOCALDATAFILE:
  {
    msgBox.setText(tr("T�rli a helyi adatb�zis f�jlt?"));
    msgBox.setInformativeText("T�rl�s hat�s�ra a legutols� friss�t�s �ta bevitt adatok elvesznek, de a k�zponti adatb�zisban t�roltak nem. Friss�t�s ut�n a legutols� t�rolt �llapot jelenik meg.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_CLEARREMOTEDATAFILE:
  {
    msgBox.setText(tr("T�rli a k�zponti adatb�zis f�jlt?"));
    msgBox.setInformativeText(tr("%1 t�rl�s�nek hat�s�ra az adatok elvesznek, de az arch�vumban t�roltak nem. Adatb�zis hiba eset�n az arch�v p�ld�ny az adatai f�jl m�sol�ssal vissza�ll�that�k.").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_CLEARREMOTEDATA:
  {
    msgBox.setText(tr("T�rli a k�zponti adatb�zis adatait?"));
    msgBox.setInformativeText(tr("%1 ki�r�t�s�nek hat�s�ra friss�t�skor a felhaszn�l�k adatai is t�rl\365dnek.").arg(textList->at(0)));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
  }
  break;
  case MSG_ERRORINLOG:
  {
    msgBox.setText(tr("A napl� f�jl hiba bejegyz�seket tartalmaz."));
    msgBox.setInformativeText(tr("Elk�ldi az adminisztr�tornak?"));
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
    text.replace(pth.pathes[PTH_remoteDatabase], "k�zponti ");
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
