#ifndef TABLEDATA_H
#define TABLEDATA_H

#include "fileaccess.h"
#include <QObject>
#include <QVector>
#include <QFile>
#include <QStringList>

class TableData : public QObject
{
  Q_OBJECT

public:
  explicit TableData(QObject* parent = 0, QString& localfile = *(QString*)"", QString& remotefile = *(QString*)"");
  ~TableData();

  bool                    loadData(void);
  void                    saveData(void);
  void                    addEntryLog(int row, int column, QString value);
  void                    eraseLog(void);
  void                    eraseData(void);
  QVector<long long int>& getMonthSums(void);
  void                    convertFileToData(QByteArray& filedata, stringMatrix& matrix);
  bool                    mergeData(void);
  bool                    getValueProperties(int row, int column, QString& date, QString& value, QString& user);

  stringMatrix            data;           // data matrix
  stringMatrix            entries;        // log of cell edits
  QVector<bool>           isWeekend;      // weekend days in columnheader
  QVector<int>            monthHeader;    // months for columns
  QVector<QString>        columnHeader;   // date headers
  QVector<QString>        rowHeader;      // business segment headers
  QVector<QString>        calculations;   // row cell calculations from column values
  int                     dbcversion;     // if different from local - data will be dropped
  int                     columns;        // number of data columns
  int                     rows;           // number of data rows
  int                     pos;            // series number of view in construction order
  static int              cnt;            // number of instances
  QString                 localFile;      // local file name of the data table
  QString                 remoteFile;     // remote file name of the data table
  bool                    isArchive;      // file is from the archives

private:
  void                    convertDataToFile(QByteArray& filedata,
                                            stringMatrix& data,
                                            stringMatrix& entries,
                                            stringArray& calculations,
                                            stringArray& rowHeader,
                                            int dbcversion);
  void                    getVersion(QByteArray& data, int& version);
  void                    correctData(stringMatrix& data);
  void                    getColumnHeader(void);
  void                    getRowHeader(stringMatrix& data, QVector<QString>& calculations, QVector<QString>& rowHeader);
  void                    getEntryLog(stringMatrix& data, stringMatrix& entries);
  QString                 createArchiveName(QString filename);
  void                    checkData(void);
  QString                 archiveFile;
  QVector<long long int>  monthSums;
  bool                    changed;
  int                     year;                                  // data year from file name

signals:
  void                    tableDataChanged(TableData*);

};

#endif // TABLEDATA_H
