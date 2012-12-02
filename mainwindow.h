#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sumtableview.h"
#include "dateselect.h"
#include "textedit.h"
#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QTextBrowser>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDate>
#include <QComboBox>
#include <QEvent>
#include <QProcess>
#include <QProgressBar>
#include <QActionGroup>

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  SumTableView*       sumTableView;
  QWidget*            mainWidget;
  QGridLayout*        layout;
  QStatusBar*         statusBar;
  QLabel*             userNameLabel;
  QLabel*             localDatabasePathLabel;
  QLabel*             remoteDatabasePathLabel;
  QTimer*             remoteAccessCheckTimer;
  QTimer*             synchroniseTimer;
  QDate               selDate;
  QLabel*             synchroniseStatus;
  Search*             searchWindow;
  int                 nextSynchronisationInSecs;

  void                createPath(QString);
  void                saveStatus(void);
  void                restoreStatus(void);
  void                updateDayTab(void);
  void                createActions(void);
  void                createMenus(void);
  void                toggleAdminAccess(void);
  bool                userKeyInput(QString*, bool*);
  void                checkErrorInLogFile();


  QMenu*              fileMenu;
  QMenu*              viewMenu;
  QMenu*              settingsMenu;
  QMenu*              administratorMenu;
  QMenu*              helpMenu;
  QMenu*              logWindowSubMenu;
  QAction*            synchroniseDatabasesAct;
  QAction*            clearLogAct;
  QAction*            showLogAct;
  QAction*            clearLocalDatabasesAct;
  QAction*            importToLocalDatabaseAct;
  QAction*            exportFromLocalDatabaseAct;
  QAction*            exitApplicationAct;
  QAction*            keyGenAct;
  QAction*            settingsEditAct;
  QAction*            setDatabasePathAct;
  QAction*            settingsSaveAct;
  QAction*            clearSelDatabaseAct;
  QAction*            generateDbcAct;
  QAction*            generateNextYearDbcsAct;
  QAction*            clearYearDatabaseAct;
  QAction*            autoUpdateAct;
  QAction*            findValueAct;
  QAction*            entryListAct;
  QAction*            hideSummaryAct;
  QAction*            editTableAsTextAct;
  QAction*            jumpDateAct;
  QAction*            showHelpAct;
  QAction*            showVersionAct;
  QAction*            switchAdminAct;
  QAction*            logWindowWarningAct;
  QAction*            logWindowInfoAct;
  QAction*            logWindowNoteAct;
  QAction*            appSettingsAct;
  QAction*            openExcelProjectAct;
  QAction*            sendMailWithSettingsAct;
  QAction*            sendMailWithLogAct;

private slots:
  void                jumpActDate(void);
  void                exitApplication();
  void                editSettings();
  void                setDatabasePath();
  void                generateKeys();
  void                synchroniseDatabases();
  void                clearLog();
  void                clearLocalDatabase();
  void                importToLocalDatabase();
  void                exportFromLocalDatabase();
  void                clearRemoteDatabaseData();
  void                generateDbc();
  void                generateNextYearDbcs();
  void                showHelp(void);
  void                showVersion(void);
  void                clearRemoteDatabase();
  void                switchAdmin();
  void                updateSettings();
  void                checkRemoteDatabase();
  void                toggleJumpThisDay(void);
  void                findValue();
  void                showEntryList();
  void                hideSummary();
  void                editTableAsText();
  void                logWindowWarning();
  void                logWindowInfo();
  void                logWindowNote();
  void                autoUpdateSlot();
  void                forceSynchroniseDatabases();
  void                setMenuStatus(void);
  void                addDatabaseAccessKey();
  void                openExcelProject();
  void                showLog();
  void                sendMailWithSettings();
  void                sendMailWithLog();
  void                searchValue(bool, QString, QString, QDateTime, QDateTime, bool);

public slots:
  void                activateTableView();
};

#endif // MAINWINDOW_H
