#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QObject>
#include "tabledata.h"
#include <QAbstractTableModel>
#include <QFile>

class TableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit TableModel(QObject* parent = 0, QString& localfile = *(QString*)"", QString& remotefile = *(QString*)"");
  ~TableModel();
  bool                loadData(QFile file);
  TableData*          tableData;
  QString             localFile;

private:
  int                 rowCount(const QModelIndex &parent = QModelIndex()) const;
  int                 columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant            data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const;
  QVariant            headerData(int section, Qt::Orientation orientation, int role) const;
  bool                setData(const QModelIndex &index, const QVariant &value, int role);
  Qt::ItemFlags       flags(const QModelIndex &index) const;

};

#endif // TABLEMODEL_H
