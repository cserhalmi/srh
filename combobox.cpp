#include "debug.h"
#include "main.h"
#include "combobox.h"
#include <QFontMetrics>
#include <QLineEdit>
#include <QFont>
#include <QStyle>
#include <QWindowsXPStyle>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent)
{
  setObjectName("ComboBox");
  this->blockSignals(true);
  this->setUpdatesEnabled(false);

  setFocusPolicy(Qt::StrongFocus);
  setEditable(false);
  QStyle* xpStyle = new QWindowsXPStyle();
  setStyle(xpStyle);
  setFont(comboBoxFont);
  updateUserSettings();
  setStyleSheet(QString("QComboBox { font: bold %1px; font-family: %2; padding: 0px 0px 0px 0px}").
                arg(comboBoxFont.pointSize()).
                arg(comboBoxFont.family()));

  connect(this, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updatePadding(int)));

  this->setUpdatesEnabled(true);
  this->blockSignals(false);
  D_CONSTRUCT("");
}

ComboBox::~ComboBox()
{
  D_DESTRUCT("");
}

void ComboBox::updateUserSettings(void)
{
  QPalette p = palette();
  p.setColor(QPalette::Highlight, cursorActiveColor);
  setPalette(p);
}

void ComboBox::updatePadding(int id)
{
  Q_UNUSED(id);
  QFontMetrics fm(font());
  QString text = currentText();
  int padding = (width() - fm.width(text) - 20) / 2;
  setStyleSheet(QString("QComboBox { font: bold %1px; font-family: %2; padding: 0px 0px 0px %3px}").
                arg(comboBoxFont.pointSize()).
                arg(comboBoxFont.family()).
                arg(padding));
}

///////////////////////////////////////////////////////////////////////////////
// OVERLOADED PROTECTED MEMBERS
///////////////////////////////////////////////////////////////////////////////

void ComboBox::resizeEvent(QResizeEvent* event)
{
  updatePadding(1);
  QWidget::resizeEvent(event);
}
