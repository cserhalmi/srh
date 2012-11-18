#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QLabel>

class ComboBox : public QComboBox
{
  Q_OBJECT
public:
  explicit ComboBox(QWidget *parent = 0);
  ~ComboBox();
  void                         updateUserSettings(void);

protected:
  void                         resizeEvent(QResizeEvent * event);

public slots:
  void                         updatePadding(int);

};

#endif // COMBOBOX_H
