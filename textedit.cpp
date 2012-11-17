#include "debug.h"
#include "main.h"
#include "textedit.h"
#include "fileaccess.h"
#include "settings.h"
#include "messages.h"
#include <QLayout>
#include <QToolBar>
#include <QCloseEvent>
#include <QTimer>
#include <QFileInfo>
#include <QDateTime>
#include <QScrollBar>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

TextEdit::TextEdit(QWidget *parent, bool tosave) : QMainWindow(parent)
{
  setObjectName("TextEdit");
  GETSILENT
  setAttribute(Qt::WA_DeleteOnClose, true);

  toSave = tosave;
  changed = false;
  textEdit              = new QTextEdit();
  mainWidget            = new QWidget(parent, Qt::Dialog);
  layout                = new QGridLayout();
  saveAct               = new QAction(this);
  exitAct               = new QAction(this);
  setWindowFlags(Qt::Tool);
  setWindowModality(Qt::NonModal);
  QToolBar* toolbar     = new QToolBar(this);
  updateTimer           = new QTimer(this);
  if (tosave)
  {
    saveAct->setShortcut(QKeySequence(tr("Ctrl+M")));
    saveAct->setText("Ctrl+M");
    saveAct->setIcon(ICON("save.png"));
  }
  else
  {
    textEdit->setReadOnly(true);
  }
  exitAct->setShortcut(QKeySequence(tr("Esc")));
  exitAct->setIcon(ICON("exit.png"));
  exitAct->setText("Esc");
  toolbar->setAllowedAreas(Qt::LeftToolBarArea);
  toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  if (tosave) toolbar->addAction(saveAct);
  toolbar->addAction(exitAct);
  toolbar->setFloatable(false);
  toolbar->setOrientation(Qt::Vertical);
  setCentralWidget(mainWidget);
  mainWidget->setLayout(layout);
  layout->addWidget(toolbar,  0, 0);
  layout->addWidget(textEdit, 0, 1);
  layout->setContentsMargins(0,0,0,0);
  layout->setMargin(0);
  setContentsMargins(0,0,0,0);
  layout->setContentsMargins(0,0,0,0);
  textEdit->setFont(textEditFont);
  textEdit->setWordWrapMode(QTextOption::NoWrap);
  textEdit->setAcceptRichText(false);
  textEdit->setTabStopWidth(1);

  connect(exitAct, SIGNAL(triggered()),
          this, SLOT(close()));
  if (tosave)
  {
    connect(saveAct, SIGNAL(triggered()),
            this, SLOT(saveText()));
    connect(textEdit, SIGNAL(textChanged()),
            this, SLOT(setChangedFlag()));
  }
  else
  {
    connect(updateTimer, SIGNAL(timeout()),
            this, SLOT(loadText()));
  }

  GETLOUD
  D_CONSTRUCT("")
}

TextEdit::~TextEdit()
{
  delete saveAct;
  delete exitAct;
  delete textEdit;
  delete layout;
  delete mainWidget;
  D_DESTRUCT("")
}

void TextEdit::saveText()
{
  if (toSave)
  {
    QByteArray t = QVariant(textEdit->toPlainText()).toByteArray();
    fileAccessed.put(t);
    if (fileAccessed.write())
    {
      changed = false;
      emit textSaved();
    }
  }
}

void TextEdit::setFile(QString filename)
{
  fileAccessed.init(filename);
  fileAccessed.modificationTimeStamp = 0;
  loadText();
}

void TextEdit::loadText()
{
  blockSignals(true);
  uint lastModified = QFileInfo(fileAccessed.pathName).lastModified().toTime_t();
  if (fileAccessed.modificationTimeStamp != lastModified)
  {
    QByteArray text = "";
    fileAccessed.modificationTimeStamp = lastModified;
    fileLoggingSuppressed = true;
    if (fileAccessed.read())
    {
      fileAccessed.get(text);
      textEdit->setText(QVariant(text).toString());
      textEdit->ensureCursorVisible();
      QTextCursor c = textEdit->textCursor();
      c.movePosition(QTextCursor::End);
      textEdit->setTextCursor(c);
      changed = false;
    }
    else
    {
      textEdit->clear();
    }
    fileLoggingSuppressed = false;
    if (!toSave) updateTimer->start(500);
  }
  blockSignals(false);
}

void TextEdit::exitTool()
{
  if (changed)
  {
    this->hide();
    if ((toSave) &&
        (Msg::pshow(MSG_QUESTION, MSG_FILE_SAVE) == QMessageBox::Save))
    {
      this->saveText();
    }
  }
  updateTimer->stop();
  this->hide();
}

void TextEdit::setChangedFlag()
{
  changed = true;
}

void TextEdit::closeEvent(QCloseEvent *event)
{ // do not exit on exit
  this->exitTool();
  event->ignore();
}

