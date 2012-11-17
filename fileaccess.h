#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include "debug.h"
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkConfigurationManager>
#include <QtNetwork/QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QUrl>

typedef QVector<QString>     stringArray;
typedef QVector<stringArray> stringMatrix;

class FileAccess : public QObject
{
  Q_OBJECT

public:
  explicit FileAccess(QString filename = "", QWidget* parent = 0);
  ~FileAccess();
  void                init(QString filename);
  bool                check();               // check availability
  bool                read();                // read from resource
  bool                write();               // write to resource
  bool                encode();              // encrypt content
  bool                decode();              // decrypt content
  QString             path();                // send file name with absolute path
  void                setpath(QString path); // set file path
  void                chomp(const char);     // removes characters from end of data
  bool                get(QByteArray& bytearray);
  bool                put(QByteArray& bytearray);

  int                 year;                  // year in file name
  QString             key;                   // file access key
  QString             fileName;              // basename
  QString             pathName;              // full path and name
  QString             segmentName;           // business segment in fimename for key generation
  uint                modificationTimeStamp; // modification TS by start

private:
  void                createFileKey();
  QByteArray          data;

};

#endif // NETWORKACCESS_H
