#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "tablemodel.h"
#include "itemdelegate.h"
#include <QTableView>
#include <QPushButton>
#include <QDate>

class TableView : public QTableView
{
  Q_OBJECT

public:
  explicit TableView(QWidget* parent = 0, QString& file = *(QString*)"");
  ~TableView();
  TableModel*            tableModel;
  QString                localFile;
  QString                remoteFile;
  QString                segment;
  QString                mappedName;

  void                   setActiveColumn(int);
  void                   hideTableView();
  void                   showTableView();
  void                   updateTableView();
  void                   editTableAsText();
  void                   updateGeometry(void);

private:
  ItemDelegate*          itemDelegate;
  QPushButton*           cornerButton;

  int                    viewPortColumns(void);
  int                    viewPortRows(void);
  bool                   getValueProperties(int row, int column, QString& date, QString& value, QString& user);

protected:
  void                   keyPressEvent(QKeyEvent *event);
  void                   currentChanged(const QModelIndex &current, const QModelIndex &previous);
  void                   focusInEvent(QFocusEvent *event);
  void                   focusOutEvent(QFocusEvent *event);

public slots:
  QString                getEntryList(QPoint, int& rows, QString& log);
  void                   showEntryList();
  void                   searchValue(bool, QString, QString, QDateTime, QDateTime, int&, int&, int&);
  void                   gotoValue(int row, int column);

private slots:
  void                   updateDataFromTextFile();
  void                   adjustSelectedColumnToScrollBarPosition(int);

signals:
  void                   tableColumnActivated(int); // sent when a table column gets active
  void                   focusArchiveList();
  void                   focusToCalendar();
  void                   deleteEntryLogScreen();

};

#endif // TABLEVIEW_H
