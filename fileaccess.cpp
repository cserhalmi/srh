#include "debug.h"
#include "fileaccess.h"
#include "messages.h"
#include "settings.h"
#include <QRegExp>
#include <QFileInfo>
#include <QByteArray>
#include <QEventLoop>
#include <QCryptographicHash>
#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

FileAccess::FileAccess(QString filename, QWidget* parent)
{
  Q_UNUSED(parent);
  setObjectName("FileAccess");

  fileName = "";
  pathName = "";
  segmentName = "";

  init(filename);

  D_CONSTRUCT("")
}

FileAccess::~FileAccess()
{
  data.clear();
  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBERS
///////////////////////////////////////////////////////////////////////////////

void FileAccess::init(QString file)
{
  fileName = QFileInfo(file).fileName();
  pathName = QFileInfo(file).absoluteFilePath();
  segmentName = QFileInfo(file).completeBaseName();
  segmentName = QFileInfo(QFileInfo(file).completeBaseName()).suffix().toAscii();
  segmentName = QFileInfo(QFileInfo(file).completeBaseName()).suffix().toAscii();
  modificationTimeStamp = QFileInfo(file).lastModified().toTime_t();
  year = QFileInfo(file).baseName().toInt();
  createFileKey();
}

bool FileAccess::write()
{
  bool ok = false;
  QFile file(pathName);
  const QByteArray d = data;
  if ((file.open(QIODevice::WriteOnly)) &&
      (file.write(d) == data.count()))
  {
    Msg::log(MSG_INFO, QString("%1 %2").arg(pathName).arg(" kiírva"));
    file.close();
    ok = true;
  }
  else
  {
    Msg::log(MSG_ERROR, QString("%1 %2").arg(pathName).arg(" kiírása sikertelen"));
  }
  return ok;
}

bool FileAccess::read()
{
  bool ok = false;
  QFile file(pathName);
  if (file.open(QIODevice::ReadOnly))
  {
    data.clear();
    data.append(file.read(file.size()));
    file.close();
    if (data.count() == file.size())
    {
      Msg::log(MSG_NOTE, QString("%1 betöltve").arg(pathName));
      ok = true;
    }
    else
    {
      Msg::log(MSG_ERROR, QString("%1 %2").arg(pathName).arg(" betöltése sikertelen"));
    }
  }
  else
  {
    Msg::log(MSG_ERROR, QString("%1 %2").arg(pathName).arg(" betöltése sikertelen"));
  }
  return ok;
}

bool FileAccess::check()
{
  QFile file(pathName);
  if (file.open(QIODevice::ReadOnly))
  {
    file.close();
    return true;
  }
  else
  {
    return false;
  }
}

bool FileAccess::encode()
{
  bool ok = false;
  data = qCompress(data);
  return ok;
}

bool FileAccess::decode()
{
  bool ok = false;
  data = qUncompress(data);
  return ok;
}

void FileAccess::chomp(const char c)
{
  while (data.endsWith(c)) data.chop(1);
}

bool FileAccess::get(QByteArray& bytearray)
{
  bytearray.clear();
  bytearray.append(data);
  return(bytearray.count() > 0);
}

bool FileAccess::put(QByteArray& bytearray)
{
  data.clear();
  data.append(bytearray);
  return(data.count() > 0);
}

QString FileAccess::path()
{
  return pathName;
}

void FileAccess::setpath(QString path)
{
  pathName = path;
  fileName = QFileInfo(path).fileName();
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
///////////////////////////////////////////////////////////////////////////////

void FileAccess::createFileKey()
{
  QByteArray ba;
  ba.append("F0F0");
  ba.append(segmentName.toUpper());
  ba.append("FFFF");
  ba.append(segmentName.toLower());
  ba.append("0F0F");
  ba = QCryptographicHash::hash(ba, QCryptographicHash::Md5);
  key = ba.toHex().toUpper();
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
///////////////////////////////////////////////////////////////////////////////
