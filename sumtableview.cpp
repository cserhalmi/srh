#include "debug.h"
#include "main.h"
#include "math.h"
#include "sumtableview.h"
#include "sumtablemodel.h"
#include "fileaccess.h"
#include "messages.h"
#include "settings.h"
#include "splashscreen.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QFileInfo>
#include <QDate>
#include <QLabel>
#include <QLineEdit>
#include <QDir>
#include <QFile>
#include <QCalendarWidget>
#include <QStyle>
#include <QWindowsXPStyle>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

SumTableView::SumTableView(QWidget *parent, QGridLayout* layout) : QTableView(parent)
{
  setObjectName("SumTableView");
  this->blockSignals(true);
  this->setUpdatesEnabled(false);

  mainWindow      = parent;
  cornerButton    = new QPushButton(this);
  yearSelect      = new ComboBox(this);
  segmentList     = new ComboBox(this);
  archiveList     = new ComboBox(this);
  sumTableModel   = new SumTableModel(this);
  calendar        = new CalendarWidget(this);
  startupLabel    = new QLabel(this);

  this->layout    = layout;
  activeTableView = NULL;

  QStyle* xpStyle = new QWindowsXPStyle();
  setStyle(xpStyle);
  horizontalHeader()->setStyle(xpStyle);
  verticalHeader()->setStyle(xpStyle);
  horizontalScrollBar()->setStyle(xpStyle);
  verticalScrollBar()->setStyle(xpStyle);
  cornerButton->setStyle(xpStyle);
  setToolTip(tr("Összesítõ"));
  cornerButton->setFocusPolicy(Qt::NoFocus);
  segmentList->setToolTip("Üzletág választó");
  archiveList->setToolTip("Archívum");
  yearSelect->setToolTip("Év választó");
  startupLabel->setStyleSheet("background-color: lightgray; color: darkgray");
  startupLabel->setFrameStyle(1);
  startupLabel->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
  startupLabel->setFont(comboBoxFont);
  startupLabel->setText(tr("Nincs hozzáférhetõ adatbázis, a következõ okok egyike miatt: "));
  startupLabel->setText(QString("%1a %2 évhez,\r\n").arg(startupLabel->text()).arg(QDate::currentDate().year()));
  startupLabel->setText(QString("%1a megadott központi adatbázis útvonalon, ").arg(startupLabel->text()));
  startupLabel->setText(QString("%1vagy a megadott kulcsokhoz nem találhatók adatbázisok.").arg(startupLabel->text()));
  layout->addWidget(this,         0, 0, 1, 4);
  layout->addWidget(yearSelect,   1, 0, 1, 2);
  layout->addWidget(segmentList,  1, 2, 1, 1);
  layout->addWidget(archiveList,  1, 3, 1, 1);
  layout->addWidget(startupLabel, 2, 0, 3, 4);
  setFrameStyle(1);
  setContentsMargins(0,0,0,0);
  setFont(tableCellFont);
  horizontalHeader()->setFont(tableHeaderFont);
  verticalHeader()->setFont(tableHeaderFont);

  selDate.setDate(QDate::currentDate().year(), QDate::currentDate().month(), QDate::currentDate().day());
  loadNewYear(QVariant(selDate.year()).toString());

  layout->addWidget(calendar, 3, 0, 1, 1);
  layout->addWidget(logBox,   3, 1, 2, 3);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  yearSelect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  segmentList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  archiveList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  startupLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  calendar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  logBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

  horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  horizontalHeader()->setToolTip(tr("Hónap figyelembevétele"));
  horizontalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
  verticalHeader()->setResizeMode(QHeaderView::Fixed);
  verticalHeader()->setToolTip(tr("Üzletág figyelembevétele"));
  verticalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionMode(QAbstractItemView::NoSelection);
  setFocusPolicy(Qt::NoFocus);
  if (activeTableView != NULL) activeTableView->setFocus();

  horizontalHeader()->setDefaultSectionSize(SCOLUMWIDHT);
  updateGeometry();

  connect(this->verticalHeader(), SIGNAL(sectionClicked(int)),
          this, SLOT(swapRowActive(int)));
  connect(this->horizontalHeader(), SIGNAL(sectionClicked(int)),
          this, SLOT(swapColumnActive(int)));
  connect(this->sumTableModel->sumTableData, SIGNAL(summedValuesUpdated()),
          this, SLOT(updateSummaryView()));
  connect(segmentList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(activateTable(int)));
  connect(archiveList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(showArchiveTable(int)));
  connect(yearSelect, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(loadNewYear(QString)));
  connect(calendar, SIGNAL(selectionChanged()),
          this, SLOT(activateTableColumn()));

  this->setUpdatesEnabled(true);
  this->blockSignals(false);
  D_CONSTRUCT("")
}

SumTableView::~SumTableView()
{
  while (tableViews.count() > 0)
  {
    delete tableViews[0];
    tableViews.remove(0, 1);
  }
  delete cornerButton;
  delete sumTableModel;
  delete archiveList;
  delete segmentList;
  delete yearSelect;
  delete calendar;

  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBERS
///////////////////////////////////////////////////////////////////////////////

void SumTableView::updateGeometry(void)
{
  cornerButton->setGeometry(1, 1, VHEADERWIDTH, SROWHEIGHT);
  segmentList->setFixedHeight(SROWHEIGHT+2*frameWidth());
  archiveList->setFixedHeight(SROWHEIGHT+2*frameWidth());
  archiveList->setFixedWidth(VHEADERWIDTH + TCOLUMWIDHT);
  yearSelect->setFixedWidth(VHEADERWIDTH+TCOLUMWIDHT);
  yearSelect->setFixedHeight(SROWHEIGHT+2*frameWidth());
  horizontalHeader()->setFixedHeight(SROWHEIGHT);
  verticalHeader()->setDefaultSectionSize(SROWHEIGHT);
  verticalHeader()->setFixedWidth(VHEADERWIDTH);
  int hasSumRow = tableViews.count() > 1 ? 1 : 0;
  setFixedHeight((sumTableModel->sumTableData->getRows()+hasSumRow)*SROWHEIGHT + frameWidth());
  setTabOrder(yearSelect, segmentList);
  setTabOrder(segmentList, archiveList);
  repaint();
}

void SumTableView::updateUserSettings(void)
{
  updateGeometry();
  segmentList->updateUserSettings();
  archiveList->updateUserSettings();
  yearSelect->updateUserSettings();
  for (int f=0; f<fileNames.count(); f++)
  {
    FileAccess file(fileNames[f]);
    sumTableModel->sumTableData->nameKeys[f] = Settings::getSetting("rowhead", file.segmentName, file.segmentName.toUpper());
  }
  activeTableView->updateTableView();
  activeTableView->updateGeometry();
  updateSummaryView();
}

void SumTableView::showSumTableViewItems(void)
{
  if (fileNames.count() == 0)
  { // change screen when no database is found
    calendar->hide();
    logBox->hide();
    if (parentWidget()->isVisible()) startupLabel->show();
    if (summaryVisible) this->hide();
  }
  else
  {
    if (parentWidget()->isVisible()) calendar->show();
    if (parentWidget()->isVisible()) logBox->show();
    if (summaryVisible) this->show();
    startupLabel->hide();
  }
}

int SumTableView::getTableCount(void)
{
  if (!tableViews.empty())
    return tableViews.count();
  else
    return 0;
}

void SumTableView::addItem(TableView* tableview)
{
  tableViews.append(tableview);
  sumTableModel->sumTableData->insertRow(sumTableModel->sumTableData->getRows() - 1, tableview->mappedName);
  connect(tableview->tableModel->tableData, SIGNAL(tableDataChanged(TableData*)),
          this->sumTableModel->sumTableData, SLOT(updateSummedValuesAndRecalculate(TableData*)));
  connect(tableview, SIGNAL(tableColumnActivated(int)),
          this, SLOT(updateCalendar(int)));
  if (tableViews.count() > 0)
  {
    int i = tableViews.count() - 1;
    connect(tableViews.at(0)->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            tableViews.at(i)->horizontalScrollBar(), SLOT(setValue(int)));
    connect(tableViews.at(i)->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            tableViews.at(0)->horizontalScrollBar(), SLOT(setValue(int)));
  }
}

TableView* SumTableView::getTableView(int i)
{
  if (i < tableViews.count())
  {
    return tableViews[i];
  }
  else
  {
    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
///////////////////////////////////////////////////////////////////////////////

//
// reload data for selected year
//
void SumTableView::forcedYearReload()
{
  loadNewYear(QVariant(selDate.year()).toString());
}

//
// loads databases for year in parameter
// and updates gui
//
void SumTableView::loadNewYear(QString year)
{
  blockSignals(true);
  setUpdatesEnabled(false);
  segmentList->blockSignals(true);
  archiveList->blockSignals(true);
  yearSelect->blockSignals(true);

  if (year.toInt() == QDate::currentDate().year())
    {selDate.setDate(year.toInt(), QDate::currentDate().month(), QDate::currentDate().day());}
  else
    {selDate.setDate(year.toInt(), 1, 1);};
  splash->message(QString("adatok betöltése %1").arg(year));
  setModel(NULL);

  segmentList->clear();
  getArchiveFiles();
  getDataFiles(year);
  archiveView = NULL;
  while (tableViews.count() > 0)
  {
    delete tableViews[0];
    tableViews.remove(0, 1);
  }
  sumTableModel->sumTableData->clear();
  for (int v=0; v<fileNames.count(); v++)
  {
    if (fileNames[v].contains(QString("%1.").arg(year)))
    {
      FileAccess file(fileNames.at(v));
      TableView* tableview = new TableView(this, file.pathName);
      tableview->mappedName = Settings::getSetting("rowhead", file.segmentName, file.segmentName.toUpper());
      layout->addWidget(tableview, 2, 0, 1, 4);
      setTabOrder(calendar, yearSelect);
      calendar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      calendar->setDateRange(QDate(QVariant(year).toInt(),1,1),
                             QDate(QVariant(year).toInt(),12,31));
      addItem(tableview);
      sumTableModel->sumTableData->updateSummedValues(tableview->tableModel->tableData);
      segmentList->addItem(tableview->mappedName);
      connect(tableview, SIGNAL(focusArchiveList()),
              this, SLOT(focusArchiveList()));
      connect(tableview, SIGNAL(focusToCalendar()),
              this, SLOT(focusCalendar()));
    }
  }
  sumTableModel->sumTableData->recalculate();
  activeTableId = 0;
  if (tableViews.count() > 0)
  {
    activateTable(0);
    tableViews.at(0)->horizontalScrollBar()->setSliderPosition(selDate.dayOfYear()-1);
    tableViews.at(0)->setActiveColumn(selDate.dayOfYear()-1);
  }
  setFixedHeight((sumTableModel->sumTableData->getRows()+1)*SROWHEIGHT + frameWidth());

  setModel(sumTableModel);
  yearSelect->setCurrentIndex(yearSelect->findText(year));
  calendar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
  logBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

  splash->finish();

  this->setVisible(summaryVisible);
  if (tableViews.count() < 2) setRowHidden(1, true); else setRowHidden(1, false);

  showSumTableViewItems();
  updateGeometry();
  segmentList->blockSignals(false);
  archiveList->blockSignals(false);
  yearSelect->blockSignals(false);
  segmentList->updatePadding(1);
  archiveList->updatePadding(1);
  yearSelect->updatePadding(1);

  D_SLOTCALL("loadNewYear")
  setUpdatesEnabled(true);
  blockSignals(false);
  emit selectedYearChanged();
}

//
// reads all datafiles and a list for the selected year
//
bool SumTableView::getDataFiles(QString year)
{
  yearSelect->clear();
  fileNames.clear();
  availableFileNames.clear();
  actualYear = year.toInt();

  QStringList nameFilter;
  nameFilter << "*.dat";
  QVector<int> dataFileYears;
  QString path;
  if (remote) path = remoteDatabasePath;
  else path = localDatabasePath;
  QDir databaseDir(path);
  QStringList dataFileList = databaseDir.entryList(nameFilter, QDir::Files, QDir::Unsorted);
  for (int i=0; i<dataFileList.count(); i++) dataFileList[i] = QString("%1/%2").arg(path).arg(dataFileList[i]);
  for (int i=0; i<dataFileList.count(); i++)
  {
    if (dataFileList[i].contains(QRegExp("/20\\d\\d.")))
    {
      availableFileNames.append(dataFileList[i]);
      QString localFileName = QString(dataFileList[i]).replace(remoteDatabasePath, localDatabasePath);
      FileAccess file(localFileName);
      if ((adminKey == correctAdminKey) || // files are visible when registry contains admin key - no need to step into admin mode
          (file.key == appSettings.value(QString("keys/%1").arg(file.segmentName), "").toString()))
      {
        if (QVariant(file.year).toString() == year)
          Msg::log(MSG_NOTE, QString("%1 adatbázis elérhetõ").arg(file.segmentName.toUpper()));
        if ((QFile(localFileName).exists() ||
             QFile(dataFileList[i]).exists()) &&
            (localFileName.contains(QString("/%1.").arg(year))) &&
            (!fileNames.contains(localFileName)))
        {
          fileNames.append(localFileName);
        }
        if (!dataFileYears.contains(file.year))
        {
          dataFileYears.append(file.year);
          yearSelect->addItem(QVariant(file.year).toString());
        }
      }
    }
  }
  return (dataFileYears.count() > 0);
}

//
// reads all archive files and a list for the selected year
//
void SumTableView::getArchiveFiles(void)
{
  archiveNames.clear();
  QStringList localFiles;
  QStringList files(getFileList(archiveDatabasePath, "dat", true));
  if (adminKey == correctAdminKey)
  { // check deleted remote files
    localFiles = getFileList(localArchiveDatabasePath, "dat", true);
    for (int f=0; f<localFiles.count(); f++)
    {
      QString localToRemote = QString(localFiles[f]).replace(localArchiveDatabasePath, archiveDatabasePath);
      if (!files.contains(localToRemote))
        Msg::log(MSG_ERROR, tr("%1 törölve lett az archívumból.").arg(localToRemote));
    }
  }
  for (int f=0; f<files.count(); f++)
  {
    FileAccess fa(files[f]);
    if (((adminKey == correctAdminKey) || // archive files are visible when registry contains admin key - no need to step into admin mode
         (fa.key == appSettings.value(QString("keys/%1").arg(fa.segmentName), "").toString())))
    {
      archiveNames.append(files[f]);
      if (adminKey == correctAdminKey)
      { // check if archive files are modified
        QString remoteToLocal = QString(localFiles[f]).replace(archiveDatabasePath, localArchiveDatabasePath);
        if (!QFile(remoteToLocal).exists())
        {
          if (QFile(files[f]).copy(remoteToLocal))
          {
            Msg::log(MSG_INFO, tr("%1 helyi másolata elkészült").arg(files[f]));
          }
        }
        else
        {
          FileAccess remoteArchive(files[f]);
          FileAccess localArchive(remoteToLocal);
          QByteArray ra;
          QByteArray la;
          remoteArchive.read();
          localArchive.read();
          remoteArchive.get(ra);
          localArchive.get(la);
          if (ra != la)
          {
            Msg::log(MSG_ERROR, tr("%1 megváltozott").arg(files[f]));
          }
        }
      }
    }
  }
}

void SumTableView::clearDatabase(bool clearremote)
{
  activeTableView->tableModel->tableData->eraseData();
  if (clearremote)
  {
    activeTableView->tableModel->tableData->dbcversion++;
    activeTableView->tableModel->tableData->saveData();
    QFile(activeTableView->remoteFile).remove();
    if (QFile::copy(activeTableView->localFile, activeTableView->remoteFile))
      Msg::log(MSG_WARNING, tr("%1 adatok törölve").arg(activeTableView->remoteFile));
    else
      Msg::log(MSG_ERROR, tr("%1 adatok törlése sikertelen").arg(activeTableView->remoteFile));
  }
  else
  {
    activeTableView->tableModel->tableData->saveData();
  }
  if (sumTableModel->sumTableData->updateSummedValues(activeTableView->tableModel->tableData))
    sumTableModel->sumTableData->recalculate();
}

bool SumTableView::removeDatabaseFile(bool clearremote)
{
  bool success = true;
  QString file(activeTableView->localFile);
  if (clearremote) file = QString(activeTableView->remoteFile);
  if (QFile(file).remove())
  {
    clearDatabase(false);
  }
  else
  {
    success = false;
    Msg::log(MSG_ERROR, QString("%1 törlése sikertelen").arg(file));
  }
  return success;
}

void SumTableView::synchroniseTables()
{
  if (remote)
  {
    bool reloadRequired = false;
    bool updateSumRequired = false;
    for (int i=0; i<tableViews.count(); i++)
    {
      if (!tableViews.at(i)->tableModel->tableData->isArchive)
      {
        if (tableViews.at(i)->tableModel->tableData->mergeData())
        {
          if (sumTableModel->sumTableData->updateSummedValues(tableViews.at(i)->tableModel->tableData))
            updateSumRequired = true;
        }
        else
        {
          reloadRequired = true;
        }
        if (tableViews.at(i)->isVisible()) tableViews.at(i)->updateTableView();
      }
      else
      {
        Msg::log(MSG_WARNING, tr("%1 nem frissítheto").arg(tableViews.at(i)->tableModel->tableData->localFile));
      }
    }
    if (reloadRequired) forcedYearReload();
    if (updateSumRequired) sumTableModel->sumTableData->recalculate();
  }
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
///////////////////////////////////////////////////////////////////////////////

void SumTableView::focusCalendar()
{
  calendar->setFocus();
}

void SumTableView::focusArchiveList()
{
  archiveList->setFocus();
}

void SumTableView::updateSummaryView()
{
  if (summaryVisible)
  {
    GETSILENT
    hide();
    show();
    GETLOUD
    D_SLOTCALL("updateSummaryView")
  }
}

void SumTableView::swapRowActive(int row)
{
  if (row < sumTableModel->sumTableData->getRows()-1)
  {
    sumTableModel->sumTableData->swapRowChecked(row);
    sumTableModel->sumTableData->recalculate();
    updateSummaryView();
  }
}

void SumTableView::swapColumnActive(int column)
{
  if (column < 12)
  {
    sumTableModel->sumTableData->swapColumnChecked(column);
    sumTableModel->sumTableData->recalculate();
    updateSummaryView();
  }
}

void SumTableView::activateTable(int id)
{
  GETSILENT
  bool sb = archiveList->signalsBlocked();
  bool sl = segmentList->signalsBlocked();
  archiveList->blockSignals(true);
  segmentList->blockSignals(true);

  int passColumn = 0;
  if (tableViews.count() > 0)
  {
    passColumn = tableViews.at(activeTableId)->selectionModel()->currentIndex().column();
  }
  for (int i=0; i<tableViews.count(); i++)
  {
    if (i == id)
    {
      tableViews.at(i)->showTableView();
      tableViews.at(i)->setFocusPolicy(Qt::StrongFocus);
      if (passColumn >= 0) tableViews.at(i)->setActiveColumn(passColumn);
      activeTableView = tableViews.at(i);
      connect(tableEditWindow, SIGNAL(textSaved()),
              activeTableView, SLOT(updateDataFromTextFile()));
    }
    else
    {
      tableViews.at(i)->setFocusPolicy(Qt::NoFocus);
      tableViews.at(i)->hideTableView();
      disconnect(tableEditWindow, SIGNAL(textSaved()),
                 tableViews.at(i), SLOT(updateDataFromTextFile()));
    }
  }
  activeTableId = id;
  sumTableModel->activeSegment = id;
  archiveList->clear();
  visibleArchiveNames.clear();
  QString cDate = QString("%1/%2/%3")
    .arg(QDate::currentDate().year())
    .arg(QString("0%1").arg(QDate::currentDate().month()).right(2))
    .arg(QString("0%1").arg(QDate::currentDate().day()).right(2));
  archiveList->addItem(cDate);
  visibleArchiveNames.append("");
  for (int v=0; v<archiveNames.count(); v++)
  {
    QRegExp rx("/(\\d+_\\d+_\\d+)/(\\d+)");
    rx.indexIn(archiveNames[v]);
    QString d = rx.cap(1).replace("_", "/");
    QString y = yearSelect->currentText();
    if ((d != cDate) &&
        (archiveNames[v].contains(QString(".%1.").arg(activeTableView->segment))))
    { // archive file for the selected year and selected segment
      archiveList->insertItem(1, d);
      visibleArchiveNames.insert(1, archiveNames[v]);
    }
  }
  updateSummaryView();
  segmentList->setCurrentIndex(id);

  segmentList->blockSignals(sl);
  archiveList->blockSignals(sb);
  D_SLOTCALL("activateTable")
  GETLOUD
}

void SumTableView::showArchiveTable(int row)
{
  GETSILENT
  segmentList->blockSignals(true);
  yearSelect->blockSignals(true);

  if (row > 0)
  { // archive view
    activeTableView->tableModel->tableData->localFile = visibleArchiveNames[row];
    activeTableView->tableModel->tableData->loadData();
    yearSelect->setEnabled(false);
    segmentList->setEnabled(false);
  }
  else
  { // active view
    activeTableView->tableModel->tableData->localFile = activeTableView->tableModel->localFile;
    activeTableView->tableModel->tableData->loadData();
    yearSelect->setEnabled(true);
    segmentList->setEnabled(true);
    setEnabled(true);
  }
  activeTableView->updateTableView();
  if (sumTableModel->sumTableData->updateSummedValues(activeTableView->tableModel->tableData))
    sumTableModel->sumTableData->recalculate();

  segmentList->blockSignals(false);
  yearSelect->blockSignals(false);

  GETLOUD
  D_SLOTCALL("showArchiveTable")
  emit archiveViewSelected();
}

void SumTableView::updateCalendar(int pos)
{
  if (!signalsBlocked())
  {
    const int month_days[2][12] = {{0u,  31u, 59u, 90u, 120u,151u,
                                    181u,212u,243u,273u,304u,334u},
                                   {0u,  31u, 60u, 91u, 121u,152u,
                                    182u,213u,244u,274u,305u,335u}};
    int leap = 0;
    int mid = 0;
    if (QDate::isLeapYear(selDate.year())) leap = 1;
    while ((pos >= month_days[leap][mid]) && (mid<12)) {mid++;} mid--;
    selDate.setDate(selDate.year(), mid + 1, pos - month_days[leap][mid] + 1);
    calendar->setSelectedDate(selDate);
  }
}

void SumTableView::activateTableColumn()
{
  if (!signalsBlocked())
  {
    selDate.setDate(calendar->selectedDate().year(),
                    calendar->selectedDate().month(),
                    calendar->selectedDate().day());
    if (activeTableView != NULL) activeTableView->setActiveColumn(selDate.dayOfYear()-1);
  }
}

