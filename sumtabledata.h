#ifndef SUMTABLEDATA_H
#define SUMTABLEDATA_H

#include <QObject>
#include <QVector>
#include "tableview.h"

class SumTableData : public QObject
{
  Q_OBJECT

public:
  explicit SumTableData(QObject* parent = 0);
  ~SumTableData();
  long long int          getData(int r, int c);
  void                   setData(int r, int c, long long int value);
  void                   insertRow(int row, QString &namekey);
  int                    getRows(void);
  bool                   getRowChecked(int row);
  bool                   getColumnChecked(int column);
  bool                   swapRowChecked(int row);
  bool                   swapColumnChecked(int column);
  QString&               getNameKey(int row);
  void                   recalculate(void);
  void                   clear(void);
  bool                   updateSummedValues(TableData*);
  QVector<QString>       nameKeys;

private:
  QVector<long long int> sumData;
  int                    columns;
  QVector<bool>          columnsChecked;
  QVector<bool>          rowsChecked;

signals:
  void                   tableDataChanged(void);
  void                   summedValuesUpdated();

public slots:
  void                   updateSummedValuesAndRecalculate(TableData*);

};

#endif // SUMTABLEDATA_H
