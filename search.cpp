#include "debug.h"
#include "main.h"
#include "search.h"
#include "fileaccess.h"
#include "settings.h"
#include "messages.h"
#include "mainwindow.h"
#include <QPushButton>
#include <QSlider>
#include <QCloseEvent>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

Search::Search(QWidget *parent) : QMainWindow(parent)
{
  setObjectName("Search");
  this->blockSignals(true);
  this->setUpdatesEnabled(false);

  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowFlags(Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
  setWindowModality(Qt::NonModal);
  setWindowTitle(tr("Keresés"));

  QWidget*     mainWidget    = new QWidget(parent, Qt::Dialog);
  QGridLayout* layout        = new QGridLayout();
  QValidator*  validatorV    = new QRegExpValidator(QRegExp("[0-9]{0,10}"), this);
  QValidator*  validatorN    = new QRegExpValidator(QRegExp("[0-9A-Za-z]{0,16}"), this);
  QLabel*      valueLabel    = new QLabel(tr("Érték"), this);
  QLabel*      nameLabel     = new QLabel(tr("Felhasználó"), this);
  QLabel*      dateLabel     = new QLabel(tr("Dátum tartomány"), this);
  QLabel*      allPagesLabel = new QLabel(tr("Minden üzletág"), this);
  QLabel*      rangeLabel    = new QLabel("-", this);
  QSlider*     transparency  = new QSlider(Qt::Horizontal, this);

  buttonNoFocus = new QPushButton(tr("&Javítás"), this);
  buttonBSearch = new QPushButton(tr("Keresés &Hátra"), this);
  buttonFSearch = new QPushButton(tr("Keresés &Elõre"), this);
  valueEdit     = new QLineEdit(this);
  nameEdit      = new QLineEdit(this);
  dateAfter     = new QDateTimeEdit(this);
  dateBefore    = new QDateTimeEdit(this);
  allPages      = new QCheckBox(this);

  setCentralWidget(mainWidget);
  mainWidget->setLayout(layout);
  rangeLabel->setAlignment(Qt::AlignCenter);
  valueLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  nameLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  valueEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  nameEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  rangeLabel->setFixedWidth(10);
  transparency->setRange(0,70);

  valueEdit->setValidator(validatorV);
  nameEdit->setValidator(validatorN);
  dateAfter->setDateTime(QDateTime(QDate(2012,1,1)));

  layout->addWidget(valueLabel,    0, 0, 1, 1);
  layout->addWidget(valueEdit,     0, 2, 1, 3);
  layout->addWidget(nameLabel,     1, 0, 1, 1);
  layout->addWidget(nameEdit,      1, 2, 1, 3);
  layout->addWidget(dateLabel,     2, 0, 1, 1);
  layout->addWidget(dateAfter,     2, 2, 1, 1);
  layout->addWidget(rangeLabel,    2, 3, 1, 1);
  layout->addWidget(dateBefore,    2, 4, 1, 1);
  layout->addWidget(allPagesLabel, 3, 0);
  layout->addWidget(allPages,      3, 1);
  layout->addWidget(transparency,  4, 0, 1, 5);
  layout->addWidget(buttonNoFocus, 5, 0);
  layout->addWidget(buttonBSearch, 5, 2);
  layout->addWidget(buttonFSearch, 5, 4);
  layout->setContentsMargins(2,2,2,2);
  layout->setMargin(4);
  layout->setSpacing(2);

  setFixedSize(370,160);

  connect(buttonNoFocus, SIGNAL(pressed()),
          parent, SLOT(activateTableView()));
  connect(buttonFSearch, SIGNAL(released()),
          this, SLOT(emitFSearch()));
  connect(buttonBSearch, SIGNAL(released()),
          this, SLOT(emitBSearch()));
  connect(transparency, SIGNAL(valueChanged(int)),
          this, SLOT(setOpacity(int)));

  this->setUpdatesEnabled(true);
  this->blockSignals(false);
  D_CONSTRUCT("")
}

Search::~Search()
{
  delete buttonFSearch;
  delete buttonBSearch;
  delete valueEdit;
  delete nameEdit;
  delete dateAfter;
  delete dateBefore;

  D_DESTRUCT("")
}

void Search::emitFSearch()
{
  emit searchValue(true, valueEdit->text(), nameEdit->text(), dateAfter->dateTime(), dateBefore->dateTime(), allPages->isChecked());
}

void Search::emitBSearch()
{
  emit searchValue(false, valueEdit->text(), nameEdit->text(), dateAfter->dateTime(), dateBefore->dateTime(), allPages->isChecked());
}

void Search::closeEvent(QCloseEvent* event)
{ // close at exit application
  hide();
  event->ignore();
}

void Search::showEvent(QShowEvent* event)
{
  move(parentWidget()->frameGeometry().left() +
       (parentWidget()->frameGeometry().width()  - frameGeometry().width()) / 2,
       parentWidget()->frameGeometry().top()  +
       (parentWidget()->frameGeometry().height() - frameGeometry().height()) / 2);
  buttonBSearch->setFocus();
  dateBefore->setDateTime(QDateTime::currentDateTime());
  event->ignore();
}

void Search::keyReleaseEvent(QKeyEvent* event)
{
  setUpdatesEnabled(false);
  if (event->key() == Qt::Key_Escape)
  {
    hide();
    event->ignore();
  }
  else
  if ((event->key() == Qt::Key_Enter) ||
      (event->key() == Qt::Key_Return))
  {
         if (buttonFSearch->hasFocus()) emitFSearch();
    else if (buttonBSearch->hasFocus()) emitBSearch();
    else if (buttonNoFocus->hasFocus()) static_cast<MainWindow*>(parentWidget())->activateTableView();
    event->ignore();
  }
  else
  {
    QMainWindow::keyReleaseEvent(event);
  }
  setUpdatesEnabled(true);
}

void Search::setOpacity(int opacity)
{
  setWindowOpacity((double)(100 - opacity) / 100);
}
