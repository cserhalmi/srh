#include "debug.h"
#include "tablemodel.h"
#include "tabledata.h"
#include "settings.h"
#include <QDir>
#include <QFont>
#include <QDate>
#include <QBrush>
#include <QIcon>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

TableModel::TableModel(QObject *parent, QString& localfile, QString& remotefile) :
  QAbstractTableModel(parent)
{
  setObjectName("TableModel");

  localFile = localfile;
  tableData = new TableData(this, localfile, remotefile);

  D_CONSTRUCT("")
}

TableModel::~TableModel()
{
  delete tableData;

  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC OVERLOADED VIRTUAL MEMBERS
///////////////////////////////////////////////////////////////////////////////

int TableModel::rowCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return MAXTABLEROWS;
}

int TableModel::columnCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return tableData->columns;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
  switch (role)
  {
  case Qt::BackgroundRole:
  {
    QBrush brush;
    if ((tableData->isWeekend[index.column()]) ||
        (index.row() >= tableData->rows))
    {
      brush.setColor(weekEndColor);
      brush.setStyle(Qt::SolidPattern);
      return brush;
    }
    else if (tableData->isArchive)
    {
      brush.setColor(archiveBackgroundColor);
      brush.setStyle(Qt::SolidPattern);
      return brush;
    }
  }
  break;
  case Qt::TextAlignmentRole:
  {
    return Qt::AlignRight + Qt::AlignVCenter;
  }
  break;
  case Qt::EditRole:
  case Qt::DisplayRole:
  {
    if ((index.row() >= tableData->rows) ||
        (index.column() >= tableData->columns)) return QVariant::Invalid;
    QString data = tableData->data[index.row()][index.column()];
    for (int i=data.length()-3; i>0; i-=3) data.insert(i, " ");
    return data;
  }
  break;
  default:
  {
    return QVariant::Invalid;
  }
  }
  return QVariant::Invalid;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  switch (role)
  {
  case Qt::EditRole:
  case Qt::DisplayRole:
  {
    tableData->addEntryLog(index.row(), index.column(), value.toString());
    tableData->saveData();
    return true;
  }
  break;
  default:
  {
    return QVariant::Invalid;
  }
  }
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Vertical) && (section >= tableData->rows))
    return QVariant::Invalid;
  if ((orientation == Qt::Horizontal) && (section >= tableData->columns))
    return QVariant::Invalid;
  switch (role)
  {
  case Qt::DecorationRole:
  {
    switch (orientation)
    {
    case Qt::Vertical:
    {
      if (tableData->calculations[section].contains("-"))
      {
        return PIXMAP("minus.png");
      }
      else
      {
        return PIXMAP("plus.png");
      }
    }
    break;
    default:
    {
      return QVariant::Invalid;
    }
    break;
    }
  }
  break;
  case Qt::DisplayRole:
  {
    switch (orientation)
    {
    case Qt::Horizontal:
    {
      if (section < tableData->columnHeader.count())
        return tableData->columnHeader[section];
      else
        return QVariant::Invalid;
    }
    break;
    case Qt::Vertical:
    {
      if (section < tableData->rowHeader.count())
        return tableData->rowHeader[section];
      else
        return QVariant::Invalid;
    }
    break;
    default:
    {
      return QVariant::Invalid;
    }
    }
  }
  break;
  default:
  {
    return QVariant::Invalid;
  }
  }
  return QVariant::Invalid;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
  if ((index.row() >= tableData->rows) ||
      (index.column() >= tableData->columns))
    return Qt::NoItemFlags;
  if (!tableData->isArchive)
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
  else
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
