#ifndef SETKEY_H
#define SETKEY_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QDateTime>
#include <QPushButton>

class SetKey : public QMainWindow
{
  Q_OBJECT

public:
  explicit SetKey(QWidget *parent = 0, QString* key = 0, bool* ok = 0);
  ~SetKey();

protected:
  void                keyReleaseEvent(QKeyEvent* event);
  void                showEvent(QShowEvent* event);

private:
  QLineEdit*          valueEdit;
  QPushButton*        buttonSetKey;
  QString*            key;
  bool*               exitSaved;

private slots:
  void                SetKeyAction();
  void                ExitAction();

};

#endif // SETKEY_H
