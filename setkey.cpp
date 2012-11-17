#include "debug.h"
#include "main.h"
#include "setkey.h"
#include "fileaccess.h"
#include "settings.h"
#include "messages.h"
#include <QPushButton>
#include <QCloseEvent>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

SetKey::SetKey(QWidget *parent, QString* key, bool* ok) : QMainWindow(parent)
{
  setObjectName("Search");
  this->blockSignals(true);
  this->setUpdatesEnabled(false);

  this->key = key;
  exitSaved = ok;
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowFlags(Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
  setWindowModality(Qt::NonModal);
  setWindowTitle("Kulcs megadása");

  QWidget*     mainWidget = new QWidget(parent, Qt::Dialog);
  QGridLayout* layout     = new QGridLayout();
  QValidator*  validator  = new QRegExpValidator(QRegExp("[0-9A-Fa-f]{32}"), this);


  buttonSetKey  = new QPushButton("&Beállítás", this);
  valueEdit     = new QLineEdit(this);

  setCentralWidget(mainWidget);
  mainWidget->setLayout(layout);

  valueEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  valueEdit->setFont(QFont("Courier", 10));
  valueEdit->setAlignment(Qt::AlignCenter);
  valueEdit->setText(*key);
  valueEdit->setValidator(validator);

  layout->addWidget(valueEdit,    0, 0);
  layout->addWidget(buttonSetKey, 1, 0);
  layout->setContentsMargins(2,2,2,2);
  layout->setMargin(2);
  layout->setSpacing(2);

  setFixedSize(280,60);
  move(parentWidget()->frameGeometry().left() +
       (parentWidget()->frameGeometry().width()  - frameGeometry().width()) / 2,
       parentWidget()->frameGeometry().top()  +
       (parentWidget()->frameGeometry().height() - frameGeometry().height()) / 2);

  valueEdit->setFocus();
  valueEdit->selectAll();

  connect(buttonSetKey, SIGNAL(released()),
          this, SLOT(SetKeyAction()));

  this->setUpdatesEnabled(true);
  this->blockSignals(false);
  D_CONSTRUCT("")
}

SetKey::~SetKey()
{
  delete buttonSetKey;
  delete valueEdit;
  D_DESTRUCT("")
}

void SetKey::SetKeyAction()
{
  (*key).clear();
  (*key).append(valueEdit->text().toUpper());
  *exitSaved = true;
  delete this;
}

void SetKey::ExitAction()
{
  delete this;
}

void SetKey::keyReleaseEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape)
  {
    event->ignore();
    delete this;
  }
  else
  if ((event->key() == Qt::Key_Enter) ||
      (event->key() == Qt::Key_Return))
  {
    if (buttonSetKey->hasFocus()) SetKeyAction();
    event->ignore();
  }
  else
  {
    QMainWindow::keyReleaseEvent(event);
  }
}

void SetKey::showEvent(QShowEvent* event)
{
  valueEdit->selectAll();
  valueEdit->setFocus();
  valueEdit->activateWindow();
  event->ignore();
}


