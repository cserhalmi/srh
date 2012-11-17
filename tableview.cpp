#include "debug.h"
#include "math.h"
#include "tableview.h"
#include "sumtableview.h"
#include "settings.h"
#include "fileaccess.h"
#include "textedit.h"
#include "messages.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QFileInfo>
#include <QMenu>
#include <QLabel>
#include <QEventLoop>
#include <QDateTime>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QStyle>
#include <QWindowsXPStyle>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

TableView::TableView(QWidget *parent, QString& localfile) : QTableView(parent)
{
  setObjectName("TableView");
  blockSignals(true);
  setUpdatesEnabled(false);

  hideTableView();
  localFile     = localfile;
  remoteFile    = QString(localfile).replace(localDatabasePath, remoteDatabasePath);
  tableModel    = new TableModel(this, localfile, remoteFile);
  cornerButton  = new QPushButton(this);
  itemDelegate  = new ItemDelegate(this);
  FileAccess file(localFile);
  segment       = file.segmentName;

  setModel(tableModel);
  setItemDelegate(itemDelegate);

  QStyle* xpStyle = new QWindowsXPStyle();
  setStyle(xpStyle);
  horizontalHeader()->setStyle(xpStyle);
  verticalHeader()->setStyle(xpStyle);
  horizontalScrollBar()->setStyle(xpStyle);
  verticalScrollBar()->setStyle(xpStyle);
  cornerButton->setStyle(xpStyle);
  setToolTip(tr("Adat táblázat"));
  setFrameStyle(1);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
  setContextMenuPolicy(Qt::CustomContextMenu);
  horizontalHeader()->setResizeMode(QHeaderView::Fixed);
  horizontalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
  cornerButton->setFocusPolicy(Qt::NoFocus);
  cornerButton->setStyleSheet("QPushButton{ color: gray }");
  cornerButton->setToolTip("Adatbázis verzió");

  verticalHeader()->setResizeMode(QHeaderView::Fixed);
  verticalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setCornerButtonEnabled(true);
  setContentsMargins(0, 0, 0, 0);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setFont(tableCellFont);
  setStyleSheet("QTableView {selection-background-color: lightGray; selection-color: black;}");
  setFont(tableCellFont);
  horizontalHeader()->setFont(tableHeaderFont);
  verticalHeader()->setFont(tableHeaderFont);

  updateGeometry();

  connect(this, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(showEntryList()));
  connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
          this, SLOT(adjustSelectedColumnToScrollBarPosition(int)));

  setUpdatesEnabled(true);
  blockSignals(false);
  D_CONSTRUCT(localFile)
}

TableView::~TableView()
{
  delete tableModel;
  delete cornerButton;
  delete itemDelegate;

  D_DESTRUCT(localFile)
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBERS
///////////////////////////////////////////////////////////////////////////////

void TableView::updateGeometry(void)
{
  horizontalScrollBar()->setMinimumHeight(TROWHEIGHT);
  setMinimumHeight((3) * TROWHEIGHT + this->horizontalScrollBar()->geometry().height() - 10);
  horizontalHeader()->setDefaultSectionSize(TCOLUMWIDHT);
  cornerButton->setGeometry(0+frameWidth(), 0+frameWidth(), VHEADERWIDTH, horizontalHeader()->height());
  if (adminKey == correctAdminKey) cornerButton->setText(QString("v%1").arg(tableModel->tableData->dbcversion));
  verticalHeader()->setDefaultSectionSize(TROWHEIGHT);
  verticalHeader()->setFixedWidth(VHEADERWIDTH);
}

void TableView::hideTableView()
{
  GETSILENT
  hide();
  GETLOUD
}

void TableView::showTableView()
{
  GETSILENT
  show();
  GETLOUD
}

void TableView::updateTableView()
{
  GETSILENT
  hide();
  show();
  GETLOUD
}

void TableView::setActiveColumn(int column)
{
  if (this != NULL)
  { // ToDo: find out how on earth got here with NULL this pointer
    QModelIndex index;
    if (selectionModel()->currentIndex().row() < 0)
    {
      index = tableModel->index(0, column);
    }
    else
    {
      index = tableModel->index(selectionModel()->currentIndex().row(), column);
    }
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
  }
}

int TableView::viewPortColumns(void)
{
  return viewport()->geometry().width() / TCOLUMWIDHT;
}

int TableView::viewPortRows(void)
{
  return viewport()->geometry().height() / TROWHEIGHT;
}

// get properties form entry list
bool TableView::getValueProperties(int row, int column, QString& date, QString& value, QString& user)
{
  int e = tableModel->tableData->entries[row].count();
  if (e == 0) return false;
  QStringList p;
  int col;
  do
  {
    e--;
    QString s = tableModel->tableData->entries[row].at(e);
    s.replace("[", ""); s.replace("]", "");
    p = s.split(";");
    col = p[1].toInt();
    date = p[0];
    value = p[2];
    user = p[3];
  } while ((e>0) && (col!=column));
  return e >= 0 ? true : false;
}

void TableView::gotoValue(int row, int column)
{
  QModelIndex index = tableModel->index(row, column);
  selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}


void TableView::searchValue(bool forward, QString value, QString user, QDateTime fromdate, QDateTime todate, int& step, int& r, int& c)
{
  step = 0;
  int cr = Math::max(selectionModel()->currentIndex().row(), 0);
  int cc = Math::max(selectionModel()->currentIndex().column(), 0);
  int ccr = cr;
  int direction = forward ? 1 : -1;
  c = cc;
  while ((c<tableModel->tableData->columns) && (c>=0) && (step == 0))
  {
    r = ccr;
    while ((r<tableModel->tableData->rows) && (r>=0) && (step == 0))
    {
      if(!tableModel->tableData->data[r][c].isEmpty())
      {
        if (((c != cc) || (r != cr)) &&
            ((value.isEmpty()) || (value == tableModel->tableData->data[r][c])))
        {
          QString euser;
          QString evalue;
          QString edatejulian;
          if (!getValueProperties(r, c, edatejulian, evalue, euser)) Msg::log(MSG_ERROR, tr("adatbázis hiba - nincs bejegyzés az értékhez (%1,%2) cellában").arg(r).arg(c));
          if (evalue != tableModel->tableData->data[r][c]) Msg::log(MSG_ERROR, tr("adatbázis hiba - (%1,%2) cella utolsó bejegyzése nem egyezik az értékkel (%3)").arg(r).arg(c).arg(evalue));
          QDateTime edate = QDateTime::fromTime_t(edatejulian.toInt());
          if (((user.isEmpty()) || (user.toLower() == euser.toLower())) &&
              (edate >= fromdate) &&
              (edate <= todate))
          {
            step = direction;
            r -= direction;
            c -= direction;
          }
        }
      }
      r += direction;
    }
    c += direction;
    ccr = forward ? 0 : tableModel->tableData->rows - 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
///////////////////////////////////////////////////////////////////////////////

void TableView::adjustSelectedColumnToScrollBarPosition(int pos)
{
  int c = selectionModel()->currentIndex().column();
  if (c >= 0)
  {
    int nc = viewPortColumns();
    if (c < pos)
    {
      setActiveColumn(pos);
    }
    else if (c > pos + nc - 1)
    {
      setActiveColumn(pos + nc - 1);
    }
  }
}

//
// tables entries for the cell pointed by parameters
// returns table and number of table rows
//
QString TableView::getEntryList(QPoint point, int& rows, QString& log)
{
  if (tableModel->tableData->entries[point.x()].count() > 0)
  {
    QRegExp rx(QString("\\[(\\d+);0{0,2}%1;(\\d+);([^\\]]+)\\]").arg(point.y()));
    rows = 0;
    for (int e=0; e<tableModel->tableData->entries[point.x()].count(); e++)
    {
      rx.indexIn(tableModel->tableData->entries[point.x()][e]);
      if (rx.cap().count() > 4)
      {
        QDateTime date = QDateTime::fromTime_t(rx.cap(1).toInt());
        QString data(rx.cap(2));
        for (int i=data.length()-3; i>0; i-=3) data.insert(i, " ");
        log.insert(0, QString("<tr><td align=\"right\" width=\"77\"><font color=\"black\">%1</font></td><td align=\"right\" width=\"125\">%2</td><td align=\"right\" width=\"100\">%3</td></tr>").
            arg(QString("%1").arg(data)).
            arg(date.toString("yyyy/MM/dd hh:mm:ss")).
            arg(QString("%1").arg(rx.cap(3))));
        rows++;
      }
    }
    log.insert(0, QString("<table cellspacing=\"0\" cellpadding=\"3.5\" frame=\"0\">"));
    log.append("</table>");
  }
  return log;
}

//
// display entry list for cell
//
void TableView::showEntryList()
{
  int ic       = selectionModel()->currentIndex().column();
  int ir       = selectionModel()->currentIndex().row();
  int eh       = 0;
  QString text = "";
  getEntryList(QPoint(ir, ic), eh, text);
  if (eh > 0)
  {
    //parentWidget()->parentWidget()->setUpdatesEnabled(false);
    QLabel*      label = new QLabel(this);
    QEventLoop*  e     = new QEventLoop(this);
    int sh   = horizontalScrollBar()->sliderPosition();
    int sv   = verticalScrollBar()->sliderPosition();
    int vc   = viewPortColumns();
    int vr   = viewPortRows();
    int ew   = 4;
    int ehpx = TROWHEIGHT * eh + 1;
    int evpx = TCOLUMWIDHT * ew + 1;
    int ihpx = columnViewportPosition(ic) + viewport()->geometry().left() - 1;
    int ivpx = rowViewportPosition(ir) + viewport()->geometry().top() - 1;
    if (ic + ew > sh + vc) ihpx -= ((ic + ew) - (sh + vc)) * TCOLUMWIDHT;
    if (ir + eh > sv + vr) ivpx -= ((ir + eh) - (sv + vr)) * TROWHEIGHT;
    label->setFixedSize(evpx, ehpx);
    label->setFont(tableCellFont);
    label->setText(text);
    label->setStyleSheet("QLabel { background-color : rgb(180,180,255); color : black; border-width: 1px; border-style: solid; border-color: black }");
    label->setTextFormat(Qt::RichText);
    label->move(ihpx, ivpx);
    //setUpdatesEnabled(true);
    connect(this, SIGNAL(deleteEntryLogScreen()), e, SLOT(quit()));
    connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), e, SLOT(quit()));
    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), e, SLOT(quit()));
    connect(this->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), e, SLOT(quit()));
    //parentWidget()->parentWidget()->setUpdatesEnabled(true);
    label->show();
    e->exec();
    delete label;
  }
  else
  {
    Msg::log(MSG_INFO, "Nincs bejegyzés a kiválasztott cellában");
  }
}

void TableView::editTableAsText()
{
  FileAccess file(localFile);
  if (adminAccess && file.read())
  {
    QByteArray text;
    file.decode();
    file.get(text);
    if (((text[text.count()-2] == '~') &&
         (QVariant(text[text.count()-1]).toInt() == tableModel->tableData->dbcversion)) ||
        (tableModel->tableData->dbcversion == 0))
    {
      if (text[text.count()-2] == '~')
        text.remove(text.count()-2, 2);
      text.replace('\000', '\t');
      text.replace('\001', '\n');
      file.put(text);
      QString tempFile(localFile);
      file.setpath(tempFile.replace(".dat", ".tmp"));
      if (file.write())
      {
        tableEditWindow->setFile(file.pathName);
        tableEditWindow->setWindowTitle(file.pathName);
        tableEditWindow->setGeometry(QRect(mapToGlobal(QPoint((frameGeometry().width() - 800)/2,
                                                              (frameGeometry().height()- 300)/2)),
                                           QSize(800, 300)));
        tableEditWindow->show();
        tableEditWindow->activateWindow();
      }
    }
    else
    {
      Msg::log(MSG_ERROR, tr("%1 verzió nem egyezik a fájlban tárol értékkel").arg(localFile));
    }
  }
}

void TableView::updateDataFromTextFile()
{
  QByteArray text;
  QString tempFile(localFile);
  FileAccess tfile(tempFile.replace(".dat", ".tmp"));
  if (tfile.read())
  {
    tfile.get(text);
    text.replace("\r", "");
    while (text.endsWith('\n')) text.chop(1);
    text.replace('\n','\001');
    text.replace('\t','\000');
    text.append('~');
    text.append((uchar)(++tableModel->tableData->dbcversion));
    FileAccess file(remoteFile); // overwrite remote instance - local will be updated as needed
    file.put(text);
    file.encode();
    if (file.write())
    {
      QFile(tempFile).remove();
      if (QFile(localFile).remove())
      {
        if (QFile::copy(remoteFile, localFile))
        {
          tableModel->tableData->loadData();
          updateGeometry();
          updateTableView();
        }
        else
        {
          Msg::log(MSG_ERROR, tr("%1 másolása sikertelen").arg(localFile));
        }
      }
      else
      {
        Msg::log(MSG_ERROR, tr("%1 eltávolítása sikertelen").arg(localFile));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// PROTECTED OVERRIDDEN VIRTUAL MEMBERS
///////////////////////////////////////////////////////////////////////////////

void TableView::focusInEvent(QFocusEvent *event)
{
  emit deleteEntryLogScreen();
  setStyleSheet(QString("QTableView {selection-background-color: %1; selection-color: black;}").arg(cursorActiveColorString));
  verticalHeader()->setStyleSheet("QHeaderView::section:checked { background-color: yellow; }");
  horizontalHeader()->setStyleSheet("QHeaderView::section:checked { background-color: yellow; }");
  QTableView::focusInEvent(event);
}

void TableView::focusOutEvent(QFocusEvent *event)
{
  emit deleteEntryLogScreen();
  setStyleSheet(QString("QTableView {selection-background-color: %1; selection-color: black;}").arg(cursorInactiveColorString));
  verticalHeader()->setStyleSheet("QHeaderView::section:checked { background-color: lightGray; }");
  horizontalHeader()->setStyleSheet("QHeaderView::section:checked { background-color: lightGray; }");
  QTableView::focusOutEvent(event);
}

void TableView::keyPressEvent(QKeyEvent *event)
{
  emit deleteEntryLogScreen();
  if (event->key() == 16777217) // TAB
  {
    emit focusToCalendar();
  }
  else if (event->key() == 16777218) // Shift+TAB
  {
    emit focusArchiveList();
  }
  else if (((event->key() > Qt::Key_9) || (event->key() < Qt::Key_0)) &&
           ((event->key() > 16777237) || (event->key() < 16777234))) // eat keys outside 0..9 and not navigation
  {
    event->ignore();
  }
  else
  {
    QTableView::keyPressEvent(event);
  }
}

void TableView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
  if ((!signalsBlocked()) &&
      (isVisible()) &&
      (current.row() >= 0) &&
      (current.column() >= 0) &&
      (current.column() != previous.column()))
  {
    emit deleteEntryLogScreen();
    emit tableColumnActivated(current.column());
  }
  QTableView::currentChanged(current, previous);
}
