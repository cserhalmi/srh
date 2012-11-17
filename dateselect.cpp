#include "debug.h"
#include "dateselect.h"
#include "fileaccess.h"
#include "settings.h"
#include "splashscreen.h"
#include <QLayout>
#include <QCloseEvent>
#include <QEventLoop>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

CalendarWidget::CalendarWidget(QWidget *parent) : QCalendarWidget(parent)
{
  setObjectName("DateTimeEdit");
  this->blockSignals(true);
  this->setUpdatesEnabled(false);

  setDateEditEnabled(false);
  setFixedWidth(VHEADERWIDTH+TCOLUMWIDHT);
  setMaximumHeight(150);
  setFocusPolicy(Qt::StrongFocus);
  setFont(calendarFont);

  QWidget *calendarNavBar = this->findChild<QWidget *>("qt_calendar_navigationbar");
  if (calendarNavBar)
  {
    QPalette pal = calendarNavBar->palette();
    pal.setColor(calendarNavBar->backgroundRole(), QColor("lightGray"));
    pal.setColor(calendarNavBar->foregroundRole(), QColor("black"));
    calendarNavBar->setPalette(pal);
  }

  connect(this, SIGNAL(clicked(QDate)),
          this, SLOT(setBkColor(QDate)));

  this->setUpdatesEnabled(true);
  this->blockSignals(false);
  D_CONSTRUCT("")
}

CalendarWidget::~CalendarWidget()
{

  D_DESTRUCT("")
}

void CalendarWidget::setBkColor(QDate date)
{
  Q_UNUSED(date);
  setStyleSheet("QCalendarWidget { selection-background-color: rgb(180,180,255) }");
}
