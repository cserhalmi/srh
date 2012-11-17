#include "debug.h"
#include "tabledata.h"
#include "sumtabledata.h"
#include "settings.h"
#include "messages.h"
#include <QString>
#include <QDate>
#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

SumTableData::SumTableData(QObject* parent) :
    QObject(parent)
{
  setObjectName("SumTableData");

  columns = 13;
  clear();

  D_CONSTRUCT("")
}

SumTableData::~SumTableData()
{
  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBERS
///////////////////////////////////////////////////////////////////////////////

void SumTableData::clear(void)
{
  sumData.clear();
  nameKeys.clear();
  rowsChecked.clear();
  columnsChecked.clear();
  columnsChecked.insert(0, columns, true);
  sumData.insert(0, columns, 0);
  rowsChecked.append(true);
  nameKeys.append("");
}

void SumTableData::insertRow(int row, QString& namekey)
{
  sumData.insert(row * columns, columns, 0);
  nameKeys.insert(row, namekey);
  rowsChecked.insert(row, true);
}

int SumTableData::getRows(void)
{
  return sumData.count() / columns;
}

bool SumTableData::getRowChecked(int row)
{
  return rowsChecked[row];
}

bool SumTableData::getColumnChecked(int column)
{
  return columnsChecked[column];
}

bool SumTableData::swapRowChecked(int row)
{
  rowsChecked[row] = !rowsChecked[row];
  return rowsChecked[row];
}

bool SumTableData::swapColumnChecked(int column)
{
  columnsChecked[column] = !columnsChecked[column];
  return columnsChecked[column];
}

long long int SumTableData::getData(int r, int c)
{
  return sumData[r * columns + c];
}

void SumTableData::setData(int r, int c, long long int value)
{
  while (sumData.count() <= r * columns + c) sumData.append(0);
  sumData[r * columns + c] = value;
}

QString& SumTableData::getNameKey(int row)
{
  return nameKeys[row];
}

void SumTableData::recalculate(void)
{
  int lastRowId = (getRows()-1)*columns;
  sumData[sumData.count()-1] = 0;
  for (int c = 0; c<12; c++)
  { // sections' month sums
    sumData[lastRowId + c] = 0;
    for (int r = 0; r<getRows()-1; r++)
    {
      if (columnsChecked[c] && rowsChecked[r])
      {
        sumData[lastRowId + c] += sumData[c + r * columns];
      }
    }
    sumData[sumData.count()-1] += sumData[lastRowId + c];
  }
  for (int r = 0; r<getRows()-1; r++)
  { // sections year sums
    sumData[(r + 1) * columns - 1] = 0;
    for (int c = 0; c<12; c++)
    {
      if (columnsChecked[c] && rowsChecked[r])
      {
        sumData[(r + 1) * columns - 1] += sumData[c + r * columns];
      }
    }
  }
  emit summedValuesUpdated();
}

bool SumTableData::updateSummedValues(TableData* tabledata)
{
  bool changed = false;
  QVector<long long int> vector = tabledata->getMonthSums();
  if (12 + columns * tabledata->pos < sumData.count())
  {
    for (int m=0; m<12; m++)
    {
      if (sumData[m + columns * tabledata->pos] != vector[m])
      {
        sumData[m + columns * tabledata->pos] = vector[m];
        changed = true;
      }
    }
  }
  else
  {
    Msg::log(MSG_ERROR, "hiba az összesíto számításnál");
  }
  return changed;
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
///////////////////////////////////////////////////////////////////////////////

void SumTableData::updateSummedValuesAndRecalculate(TableData* tabledata)
{
  updateSummedValues(tabledata);
  recalculate();
}

