#ifndef PATHES_H
#define PATHES_H
#include <QString.h>
#include <QStringList>

enum AllPathes
{
  PTH_installer,
  PTH_application,
  PTH_installedApplication,
  PTH_remoteDatabase,
  PTH_archiveDatabase,
  PTH_working,
  PTH_localDatabase,
  PTH_localArchiveDatabase,
  PTH_importExport,
  PTH_count,
  FLE_help = PTH_count,
  FLE_settings,
  FLE_log,
  FLE_count
};

class Pathes
{
public:
  Pathes();
  QStringList pathes;
  bool checkPathes(void);
  QString getLog(void);
  void createPath(QString path);

private:
  bool pathesAreOk;

  void createUserPathes(void);

};

#endif // PATHES_H
