#ifndef SUMTABLEMODEL_H
#define SUMTABLEMODEL_H

#include "sumtabledata.h"
#include <QAbstractTableModel>

class SumTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit SumTableModel(QObject* parent = 0);
  ~SumTableModel();
  SumTableData*       sumTableData;
  int                 activeSegment;

private:
  int                 rowCount(const QModelIndex &parent = QModelIndex()) const;
  int                 columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant            data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const;
  QVariant            headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // SUMTABLEMODEL_H
