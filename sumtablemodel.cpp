#include "debug.h"
#include "sumtablemodel.h"
#include "settings.h"
#include <QDir>
#include <QFont>
#include <QDate>
#include <QBrush>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

SumTableModel::SumTableModel(QObject *parent) : QAbstractTableModel(parent)
{
  setObjectName("SumTableModel");

  activeSegment = 0;
  sumTableData  = new SumTableData(this);

  D_CONSTRUCT("")
}

SumTableModel::~SumTableModel()
{

  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC OVERLOADED VIRTUAL MEMBERS
///////////////////////////////////////////////////////////////////////////////

int SumTableModel::rowCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return sumTableData->getRows();
}

int SumTableModel::columnCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return 13;
}

QVariant SumTableModel::data(const QModelIndex &index, int role) const
{
  switch (role)
  {
  case Qt::BackgroundRole:
  {
    if ((index.column() > 11) ||
        (index.row() > sumTableData->getRows()-2))
    {
      QBrush brush;
      brush.setColor(weekEndColor);
      brush.setStyle(Qt::SolidPattern);
      return brush;
    }
    else
    {
      if ((!sumTableData->getRowChecked(index.row())) ||
          (!sumTableData->getColumnChecked(index.column())))
      {
        QBrush brush;
        brush.setColor(weekEndColor);
        brush.setStyle(Qt::SolidPattern);
        return brush;
      }
      else if (index.row() == activeSegment)
      {
        QBrush brush;
        brush.setColor(archiveBackgroundColor);
        brush.setStyle(Qt::SolidPattern);
        return brush;
      }
    }
    return QVariant::Invalid;
  }
  break;
  case Qt::TextAlignmentRole:
  {
    return Qt::AlignRight + Qt::AlignVCenter;
  }
  break;
  case Qt::DisplayRole:
  {
    if ((sumTableData->getRowChecked(index.row())) &&
        (sumTableData->getColumnChecked(index.column())) &&
        (sumTableData->getData(index.row(), index.column())))
    {
      QString data = QString("%1").arg(sumTableData->getData(index.row(), index.column()));
      for (int i=data.length()-3; i>0; i-=3) data.insert(i, " ");
      return data;
    }
    return QVariant::Invalid;
  }
  break;
  default:
    return QVariant::Invalid;
  }
  return QVariant::Invalid;
}

QVariant SumTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  switch (role)
  {
  case Qt::DecorationRole:
  {
    switch (orientation)
    {
    case Qt::Vertical:
    {
      if (section < sumTableData->getRows()-1)
      {
        if (sumTableData->getRowChecked(section))
        {
          return PIXMAP("on.png");
        }
        else
        {
          return PIXMAP("off.png");
        }
      }
      else
      {
        return PIXMAP("sum.png");
      }
    }
    break;
    case Qt::Horizontal:
    {
      if (section < 12)
      {
        if (sumTableData->getColumnChecked(section))
        {
          return PIXMAP("on.png");
        }
        else
        {
          return PIXMAP("off.png");
        }
      }
      else
      {
        return PIXMAP("sum.png");
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
      if (section < 12)
      {
        QLocale local;
        return local.monthName(section+1);
      }
    }
    break;
    case Qt::Vertical:
    {
      return sumTableData->getNameKey(section);
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
    return QVariant::Invalid;
  }
  return QVariant::Invalid;
}
