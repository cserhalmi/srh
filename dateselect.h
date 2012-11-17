#ifndef DATESELECT_H
#define DATESELECT_H

#include <QDateTimeEdit>
#include <QCalendarWidget>

class CalendarWidget : public QCalendarWidget
{
  Q_OBJECT

public:
  explicit CalendarWidget(QWidget* parent);
  ~CalendarWidget();

private slots:
  void setBkColor(QDate);

};

#endif // DATESELECT_H
