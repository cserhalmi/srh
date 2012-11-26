#include "debug.h"
#include "main.h"
#include "tabledata.h"
#include "splashscreen.h"
#include "settings.h"
#include "messages.h"
#include "math.h"
#include <QString>
#include <QVariant>
#include <QDate>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

int TableData::cnt = 0;

TableData::TableData(QObject* parent, QString& localfile, QString& remotefile) : QObject(parent)
{
  setObjectName("TableData");

  pos         = cnt;
  cnt++;
  changed     = false;
  localFile   = localfile;
  remoteFile  = remotefile;
  archiveFile = createArchiveName(localfile);
  year        = QFileInfo(localFile).baseName().left(4).toInt();
  columns     = QDate(year, 1, 1).daysInYear();
  rows        = 0;
  dbcversion  = 0;
  isArchive   = localfile.contains("/archive/");
  if ((!remote) || (isArchive))
  {
    loadData();
  }
  else
  {
    mergeData();
  }

  D_CONSTRUCT("")
}

TableData::~TableData()
{
  cnt--;
  if (!isArchive) saveData();

  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
///////////////////////////////////////////////////////////////////////////////

//
// decompose file bytearray to string matrix
//
void TableData::convertFileToData(QByteArray& filedata, stringMatrix& data)
{
  data.clear();
  while (filedata.endsWith('\001')) filedata.chop(1);
  QList<QByteArray> lines = filedata.split('\001');
  for (int l=0; l<lines.count(); l++)
  {
    QList<QByteArray> fields = lines[l].split('\000');
    QVector<QString> sfields;
    for (int f=0; f<fields.count(); f++) sfields.append(fields[f]);
    while (sfields.count() < columns) sfields.append("");
    data.append(sfields);
  }
}

//
// compose table data parts to bytearray to be written to file
//
void TableData::convertDataToFile(QByteArray& filedata,
                                  stringMatrix& data,
                                  stringMatrix& entries,
                                  stringArray& calculations,
                                  stringArray& rowHeader,
                                  int version)
{
  filedata.clear();
  for (int r=0; r<data.count(); r++)
  {
    filedata.append(QVariant(QString("(%1) %2").arg(calculations[r]).arg(rowHeader[r])).toByteArray());
    filedata.append('\000');
    for (int c=0; c<data[r].count(); c++)
    {
      filedata.append(data[r][c]);
      filedata.append('\000');
    }
    for (int c=0; c<entries[r].count(); c++)
    {
      filedata.append(entries[r][c]);
      filedata.append('\000');
    }
    filedata.remove(filedata.length()-1, 1);
    filedata.append('\001');
  }
  filedata.remove(filedata.length()-1, 1);
  filedata.append('~');
  filedata.append(QVariant(version).toChar());
}

//
// complete row cell number to column count
// fill first cell with defaul row header
//
void TableData::correctData(stringMatrix& data)
{
  for (int r=1; r<data.count(); r++)
  {
    while (data[r].count() <= columns)
      data[r].append("");
    if (data[r][0].isEmpty())
      data[r][0] = QString("(+) sor %1").arg(r);
  }
}

//
// create path and name for arhive instance
//
QString TableData::createArchiveName(QString filename)
{
  QDate cd = QDate::currentDate();
  QString d = QString("%1/%2_%3_%4/")
      .arg(pth.pathes[PTH_archiveDatabase])
      .arg((QString("00%1").arg(cd.year())).right(4))
      .arg((QString("0%1").arg(cd.month())).right(2))
      .arg((QString("0%1").arg(cd.day())).right(2));
  if ((!QFileInfo(d).isDir()) &&
      (QDir(pth.pathes[PTH_remoteDatabase]).exists()))
  {
    QDir dir;
    if (!dir.mkpath(d))
    {
      Msg::log(MSG_ERROR, QString("az archív útvonal \"%1\" nem hozható létre").arg(d));
      return "";
    }
  }
  d.append(QFileInfo(filename).fileName());
  return d;
}

//
// create column header with tera days and weekend flags
//
void TableData::getColumnHeader(void)
{
  QLocale local;
  QDate date(year, 1, 1);
  monthHeader.clear();
  columnHeader.clear();
  isWeekend.clear();
  for (int m=1; m<=12; m++)
  {
    date.setDate(year, m, 1);
    for (int d=1; d<=date.daysInMonth(); d++)
    {
      date.setDate(year, m, d);
      monthHeader.append(m-1);
      int dow = date.dayOfWeek();
      if (dow > 5) isWeekend.append(true); else isWeekend.append(false);
      columnHeader.append(QString("%1/%2 %3").
        arg(m).arg(d).arg(local.dayName(date.dayOfWeek(), QLocale::ShortFormat)));
    }
  }
}

//
// substract entry log from table data rows
//
void TableData::getEntryLog(stringMatrix& data, stringMatrix& entries)
{
  entries.clear();
  QVector<QString> row;
  for (int r=0; r<data.count(); r++)
  {
    int c = columns+1;
    entries.append(row);
    while (c < data[r].count())
    {
      if (data[r][c].contains("["))
      {
        QString e = data.at(r).at(c).mid(1,data.at(r).at(c).length()-2);
        QStringList l = e.split(";");
        if (l.at(0).toLongLong() < 1000)
        { // old style entry log detected and converted to new
          data[r][c] = QString("[%1;%2;%3;%4]").
              arg(l[2]).arg(l[0]).arg(l[1]).arg(l[3]);
        }
        entries[r].append(data[r][c]);
      }
      data[r].remove(c, 1);
    }
  }
}

//
// get database version or let it be zero
//
void TableData::getVersion(QByteArray& ba, int& version)
{
  version = 0;
  if (ba[ba.count()-2] == '~')
  {
    version = ba[ba.count()-1];
    ba.remove(ba.count()-2, 2);
  }
}

//
// decompose first row cells to
// row header and calculation and
// substract data entry log
//
void TableData::getRowHeader(stringMatrix& data, QVector<QString>& calculations, QVector<QString>& rowHeader)
{
  rows = data.count();
  calculations.clear();
  rowHeader.clear();
  for (int r=0; r<rows; r++)
  {
    QRegExp rx("\\(([^\\)]+)\\)\\s*(\\S(.+))");
    rx.indexIn(data[r][0]);
    if (rx.cap().count() > 2)
    {
      calculations.append(rx.cap(1));
      rowHeader.append(rx.cap(2));
    }
    else
    {
      calculations.append("");
      rowHeader.append(data[r][0]);
    }
    data[r].remove(0, 1);
  }
}

// get properties form entry list
bool TableData::getValueProperties(int row, int column, QString& date, QString& value, QString& user)
{
  int e = entries[row].count();
  if (e == 0) return false;
  QStringList p;
  int col;
  do
  {
    e--;
    QString s = entries[row].at(e);
    s.replace("[", ""); s.replace("]", "");
    p = s.split(";");
    col = p[1].toInt();
    date = p[0];
    value = p[2];
    user = p[3];
  } while ((e>0) && (col!=column));
  return e >= 0 ? true : false;
}

void TableData::checkData(void)
{
  for (int c=0; c<columns; c++)
  {
    for (int r=0; r<rows; r++)
    {
      if(!data.at(r).at(c).isEmpty())
      {
        QString euser;
        QString evalue;
        QString edatejulian;
        if (!getValueProperties(r, c, edatejulian, evalue, euser)) Msg::log(MSG_ERROR, tr("adatbázis hiba - nincs bejegyzés az értékhez (%1,%2) cellában").arg(r).arg(c));
        if (evalue != data[r][c]) Msg::log(MSG_ERROR, tr("adatbázis hiba - (%1,%2) cella utolsó bejegyzése nem egyezik az értékkel (%3)").arg(r).arg(c).arg(evalue));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBERS
///////////////////////////////////////////////////////////////////////////////

bool TableData::loadData(void)
{
  FileAccess file(localFile);
  isArchive = localFile.contains("/archive/");
  if (file.read())
  {
    file.decode();
    file.chomp('\001');
    QByteArray ba;
    file.get(ba);
    getVersion(ba, dbcversion);
    convertFileToData(ba, data);
    correctData(data);
    getColumnHeader();
    getEntryLog(data, entries);
    getRowHeader(data, calculations, rowHeader);
    checkData();
    emit tableDataChanged(this);
    return true;
  }
  return false;
}

//
// save local data if changed
//
void TableData::saveData(void)
{
  if ((changed) ||
      (settings.contains("resetversions"))
)
  {
    FileAccess file(localFile);
    QByteArray ba;
    if (settings.contains("resetversions")) dbcversion = 0;
    convertDataToFile(ba, data, entries, calculations, rowHeader, dbcversion);
    file.put(ba);
    file.encode();
    if (file.write()) changed = false;
  }
}

//
// merge entries of file1 and file2
// and copy result to both files
//
// file1 is the master means that if a row is
// deleted it will not be put back from file2
//
// if file2 does not exist it is created from
// file1 by copying
//
// if file1 does not exist file2 is deleted
// with an error message
//
// usually sychronises remote and local files
// by creating local instance by first run
// then merging local and remote data changing
// both locations
//
bool TableData::mergeData(void)
{
  bool merged = false;
  bool remoteChanged = false;
  isArchive = localFile.contains("/archive/");
  if (!QFile(localFile).exists())
  { // create file from repo
    if (QFile::copy(remoteFile, localFile))
    {
      Msg::log(MSG_INFO, QString("%1 helyi adatbázisba másolva").arg(remoteFile));
      loadData();
      return true;
    }
    else
    {
      Msg::log(MSG_INFO, QString("%1 helyi adatbázisba másolása sikertelen").arg(remoteFile));
      return false;
    }
  }
  if ((remote) &&
      (!QFile(remoteFile).exists()) &&
      (QDir(QFileInfo(remoteFile).absoluteDir()).exists()))
  { // delete local file that is removed from repo although repo is available
    if (QFile(localFile).remove())
    {
      Msg::log(MSG_ERROR, QString("%1 nem található a központi adatbázisban - törölve a helyi adatbázisból").arg(remoteFile));
      return false;
    }
    else
    {
      return false;
    }
  }
  FileAccess f[2];
  f[0].init(remoteFile);
  f[1].init(localFile);
  stringMatrix d[2]; // data
  int v[2]; // version
  stringMatrix e[2]; // entries
  QByteArray b[2]; // file bytearrays
  QVector<QString> c[2]; // row calculations
  QVector<QString> h[2]; // row headers
  QHash<QString, int> p[2]; // rowheader(position) hash
  for (int i=0; i<2; i++)
  { // separate data, formula, header and entries for both files
    if (!f[i].read()) return false;
    f[i].decode();
    f[i].get(b[i]);
    getVersion(b[i], v[i]);
    convertFileToData(b[i], d[i]);
    getEntryLog(d[i], e[i]);
    getRowHeader(d[i], c[i], h[i]);
    for (int j=0; j<h[i].count(); j++)
    {
      if (p[i].contains(h[i][j]))
        Msg::log(MSG_ERROR, QString("%1 sor fejléc nem egyedi").arg(h[i][j]));
      p[i][h[i][j]] = j;
    }
  }
  if ((v[0] != v[1]) ||
      (h[0] != h[1]) ||
      (c[0] != c[1]))
  { // administrator modified central database
    d[1] = d[0];
    e[1] = e[0];
    h[1] = h[0];
    c[1] = c[0];
    if ((v[0] != v[1])) Msg::log(MSG_NOTE, tr("%1 -> %2 adatbázis verzió változás").arg(v[0]).arg(v[1]));
    if ((h[0] != h[1])) Msg::log(MSG_NOTE, tr("adatbázis sor felirat, vagy struktúra változás"));
    if ((c[0] != c[1])) Msg::log(MSG_NOTE, tr("adatbázis sor elõjel, vagy számítás változás"));
    remoteChanged = true;
  }
  else
  { // clear data if versions do not match
    QHashIterator<QString, int> hi(p[0]);
    while (hi.hasNext())
    { // iterate through file1 rowname(position) hash
      hi.next();
      if (p[1].contains(hi.key()))
      { // file2 has row named as in file1
        int r0 = p[0][hi.key()];
        int r1 = p[1][hi.key()];
        if (e[0][r0] != e[1][r1])
        { // entries are different in file1 row and file2 row
          for (int j=0; j<e[1][r1].count(); j++)
            if (!e[0][r0].contains(e[1][r1][j]))
            {
              e[0][r0].append(e[1][r1][j]);
            }
          qSort(e[0][r0]); // merged entries sorted - 1)time 2)column
          merged = true;
          for (int r=0; r<e[0][r0].count(); r++)
          {
            QStringList s = e[0][r0].at(r).split(";");
            int c = s[1].toInt();
            d[0][r0][c] = s[2];
          }
        }
      }
    }
    if (merged)
    {
      convertDataToFile(b[0], d[0], e[0], c[0], h[0], v[0]);
      bool i = outputLogInfosFlag;
      bool n = outputLogNotesFlag;
      outputLogNotesFlag = false;
      outputLogInfosFlag = false;
      for (int i=0; i<2; i++)
      { // put b[0] array to both files
        f[i].put(b[0]);
        f[i].encode();
        f[i].write();
        Msg::log(MSG_WARNING, QString("%1 frissítve").arg(f[i].pathName));
      }
      f[1].setpath(archiveFile);
      f[1].write();
      Msg::log(MSG_NOTE, QString("%1 archiválva").arg(f[i].pathName));
      outputLogInfosFlag = i;
      outputLogNotesFlag = n;
    }
    else
    {
      Msg::log(MSG_INFO, QString("helyi és %1 adatbázisok megegyeznek").arg(remoteFile));
    }
  }
  entries = e[0];
  data = d[0];
  rowHeader = h[0];
  calculations = c[0];
  dbcversion = v[0];
  rows = h[0].count();
  getColumnHeader();
  if (remoteChanged)
  {
    Msg::log(MSG_ERROR, tr("%1 adatokat az adminisztrátor módosította").arg(remoteFile));
    changed = true;
    saveData();
  }
  checkData();
  return true;
}

//
// add formatted entry log by data input
//
void TableData::addEntryLog(int row, int column, QString value)
{
  QString tvalue = value
      .replace(QRegExp("[^\\d]+"), "")
      .replace(QRegExp("^0+"), "")
      .right(10);
  if ((tvalue.isEmpty()) && (data[row][column] != "")) tvalue="0";
  if (data[row][column] != tvalue)
  {
    changed = true;
    data[row][column] = tvalue;
    for (int i=tvalue.length()-3; i>0; i-=3) tvalue.insert(i, " ");
    tvalue.replace(" ","");
    entries[row].append(QString("[%1;%2;%3;%4]").
                        arg(QDateTime::currentDateTime().toTime_t()).
                        arg(QString("00%1").arg(column).right(3)).arg(tvalue).
                        arg(userName));
    emit tableDataChanged(this);
  }
}

void TableData::eraseLog(void)
{
  for (int r=0; r<rows; r++)
  {
    entries[r].clear();
  }
  changed = true;
}

void TableData::eraseData(void)
{
  eraseLog();
  for (int r=0; r<rows; r++)
  {
    for (int c=0; c<columns; c++)
    {
      data[r][c] = "";
    }
  }
  changed = true;
}

QVector<long long int>& TableData::getMonthSums(void)
{
  monthSums.fill(0, 12);
  for (int c=0; c<columns; c++)
  {
    for (int r=0; r<rows; r++)
    {
      int sign = calculations[r].contains("-") ? -1 : 1;
      monthSums[monthHeader[c]] += sign * data[r][c].toLongLong();
    }
  }
  return monthSums;
}


