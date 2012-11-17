#ifndef SUMTABLEVIEW_H
#define SUMTABLEVIEW_H

#include <QObject>
#include "sumtablemodel.h"
#include "tableview.h"
#include "combobox.h"
#include "dateselect.h"
#include "search.h"

#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QScrollBar>
#include <QTimer>
#include <QDate>
#include <QGridLayout>

class SumTableView : public QTableView
{
  Q_OBJECT

public:
  explicit SumTableView(QWidget* parent = 0, QGridLayout* layout = 0);
  ~SumTableView();
  SumTableModel*      sumTableModel;
  TableView*          getTableView(int i);
  ComboBox*           yearSelect;
  ComboBox*           archiveList;
  QVector<QString>    fileNames;
  QVector<QString>    availableFileNames;
  QVector<QString>    archiveNames;
  QCalendarWidget*    calendar;
  QGridLayout*        layout;
  TableView*          activeTableView;
  QVector<TableView*> tableViews;

  int                 activeTableId;
  int                 actualYear;
  int                 getTableCount(void);
  void                clearDatabase(bool clearremote);
  void                synchroniseTables();
  bool                removeDatabaseFile(bool clearremote);
  void                showSumTableViewItems(void);
  void                updateUserSettings(void);
  void                updateGeometry(void);

private:
  QWidget*            mainWindow;
  QDate               selDate;
  QPushButton*        cornerButton;
  ComboBox*           segmentList;
  TableView*          archiveView;
  QLabel*             startupLabel;
  QVector<QString>    rowNames;
  QVector<QString>    visibleArchiveNames;

  void                addItem(TableView* tableview);
  bool                getDataFiles(QString year);
  void                getArchiveFiles(void);
  void                forcedYearReload();

private slots:
  void                updateCalendar(int);
  void                focusCalendar();
  void                focusArchiveList();
  void                activateTableColumn();

public slots:
  void                swapRowActive(int);
  void                swapColumnActive(int);
  void                loadNewYear(QString);
  void                activateTable(int);
  void                showArchiveTable(int);
  void                updateSummaryView();

signals:
  void                selectedYearChanged();
  void                archiveViewSelected();

};

#endif // SUMTABLEVIEW_H
