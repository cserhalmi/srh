#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QMainWindow>
#include <QTimer>
#include <QGridLayout>
#include <QLabel>

class SplashScreen : public QMainWindow
{
  Q_OBJECT

public:
  explicit SplashScreen(QWidget *parent = 0);
  ~SplashScreen();
  void                finish(void);
  void                message(QString text = "", bool empty = false);
  QWidget*            container;

private:
  QWidget*            mainWidget;
  QGridLayout*        layout;
  QTimer*             fadeTimer;
  QLabel*             label;
  int                 timer;
  bool                startup;

private slots:
  void                fade();

};

#endif // SPLASHSCREEN_H
