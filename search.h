#ifndef SEARCH_H
#define SEARCH_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QDateTime>
#include <QPushButton>
#include <QCheckBox>

class Search : public QMainWindow
{
  Q_OBJECT

public:
  explicit Search(QWidget *parent = 0);
  ~Search();

protected:
  void                closeEvent(QCloseEvent* event);
  void                showEvent(QShowEvent* event);
  void                keyReleaseEvent(QKeyEvent* event);

private:
  QLineEdit*          valueEdit;
  QLineEdit*          nameEdit;
  QDateTimeEdit*      dateAfter;
  QDateTimeEdit*      dateBefore;
  QPushButton*        buttonFSearch;
  QPushButton*        buttonBSearch;
  QPushButton*        buttonNoFocus;
  QCheckBox*          allPages;

private slots:
  void                emitFSearch();
  void                emitBSearch();
  void                setOpacity(int);


signals:
  void                searchValue(bool, QString, QString, QDateTime, QDateTime, bool);

};

#endif // SEARCH_H
