#include "debug.h"
#include "splashscreen.h"
#include "settings.h"
#include <QPixmap>
#include <QApplication>

#define SPLASHWIDTH 370
#define SPLASHEIGHT 170

SplashScreen::SplashScreen(QWidget *parent) : QMainWindow(parent)
{
  setObjectName("SplashScreen");

  container         = parent;
  mainWidget        = new QWidget(parent, Qt::Dialog);
  layout            = new QGridLayout();
  fadeTimer         = new QTimer(this);
  label             = new QLabel();
  startup           = true;
  layout->setSpacing(0);
  layout->setMargin(0);
  layout->setContentsMargins(0,0,0,0);
  setCentralWidget(mainWidget);
  mainWidget->setLayout(layout);
  layout->addWidget(label);
  label->setFont(QFont("Tahoma", 18));
  label->setStyleSheet("QLabel { background-color : white; color : blue; border-width: 1px; border-style: solid; border-color: black}");
  label->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setAttribute(Qt::WA_DontShowOnScreen, false);
  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
  setFixedSize(370,170);
  setWindowOpacity(0.75);

  connect(fadeTimer, SIGNAL(timeout()),
          this, SLOT(fade()));

  D_CONSTRUCT("")
}

SplashScreen::~SplashScreen()
{
  delete fadeTimer;
  delete label;
  delete layout;
  delete mainWidget;

  D_DESTRUCT("")
}

void SplashScreen::finish(void)
{
  fadeTimer->start(10);
}

void SplashScreen::message(QString text, bool empty)
{
  if (startup)
  { // startup screen center
    move((desktop->screenGeometry(0).width() - SPLASHWIDTH) / 2,
         (desktop->screenGeometry(0).height() - SPLASHEIGHT) / 2);
  }
  else
  {
    move(container->frameGeometry().left() + (container->frameGeometry().width() - SPLASHWIDTH) / 2,
         container->frameGeometry().top()  + (container->frameGeometry().height() - SPLASHEIGHT) / 2);
  }
  timer = 0;
  if (label->text() != text)
  {
    QString t;
    if ((empty) || (label->text().isEmpty()))
      t = text;
    else
      t = QString("%1\n%2").arg(label->text()).arg(text);
    setWindowOpacity(0.75);
    label->setText(t);
    show();
  }
  repaint();
}

void SplashScreen::fade()
{
  if (timer < 40)
  {
    timer++;
  }
  else
  {
    if (windowOpacity() > 0)
    {
      setWindowOpacity(windowOpacity() - 0.01);
    }
    else
    {
      label->clear();
      startup = false;
    }
  }
}
