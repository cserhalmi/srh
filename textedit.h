#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "fileaccess.h"
#include <QMainWindow>
#include <QTextEdit>
#include <QGridLayout>
#include <QAction>

class TextEdit : public QMainWindow
{
  Q_OBJECT

public:
  explicit TextEdit(QWidget *parent = 0, bool tosave = false);
  ~TextEdit();
  QTextEdit*          textEdit;
  void                setFile(QString filename);

private:
  QWidget*            mainWidget;
  QGridLayout*        layout;
  QAction*            exitAct;
  QAction*            saveAct;
  QMenu*              menu;
  QTimer*             updateTimer;
  FileAccess          fileAccessed;
  bool                changed;
  bool                toSave;
  void                closeEvent(QCloseEvent *event);

private slots:
  void                saveText();
  void                exitTool();
  void                setChangedFlag();

public slots:
  void                loadText();

signals:
  void                textSaved();

};

#endif // TEXTEDIT_H
