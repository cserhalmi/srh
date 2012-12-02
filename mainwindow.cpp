#include "debug.h"
#include "mainwindow.h"
#include "main.h"
#include "math.h"
#include "splashscreen.h"
#include "messages.h"
#include "settings.h"
#include "fileaccess.h"
#include "textedit.h"
#include "search.h"
#include "setkey.h"
#include "dateselect.h"

#include <QDesktopServices>
#include <QSettings>
#include <QStatusBar>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QDir>
#include <QHeaderView>
#include <QScrollBar>
#include <QFont>
#include <QVector>
#include <QLocale>
#include <QScrollBar>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QLineEdit>
#include <QToolBar>
#include <QPalette>
#include <QTimer>
#include <QUrlInfo>
#include <QInputDialog>
#include <QMotifStyle>
#include <QFileDialog>
#include <QPointer>

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCT AND DESTRUCT
///////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  setObjectName("MainWindow");
  this->blockSignals(true);
  this->setUpdatesEnabled(false);

  restoreStatus();
  Msg::saveStartupStateToLogFile();

  selDate.setDate(QDate::currentDate().year(),
                  QDate::currentDate().month(),
                  QDate::currentDate().day());

  mainWidget                = new QWidget(this);
  layout                    = new QGridLayout(mainWidget);
  statusBar                 = new QStatusBar(this);
  remoteDatabasePathLabel   = new QLabel("");
  localDatabasePathLabel    = new QLabel("");
  userNameLabel             = new QLabel(userName);
  remoteAccessCheckTimer    = new QTimer(this);
  synchroniseTimer          = new QTimer(this);
  synchroniseStatus         = new QLabel(QString("> %1 >").arg(AUTOUPDATEPERIOD));
  settingsWindow            = new TextEdit(this, true);
  logWindow                 = new TextEdit(this, false);
  tableEditWindow           = new TextEdit(this, true);
  searchWindow              = new Search(this);
  sumTableView              = NULL;
  nextSynchronisationInSecs = AUTOUPDATEPERIOD;

  setAttribute(Qt::WA_AlwaysShowToolTips, true);
  setWindowIcon(ICON("logo.png"));
  setWindowTitle("CashFlow");
  setCentralWidget(mainWidget);
  setStatusBar(statusBar);
  mainWidget->setLayout(layout);
  layout->setSpacing(0);
  layout->setMargin(0);
  layout->setContentsMargins(2, 2, 2, 0);
  createActions();
  createMenus();
  logBox->setFont(logWindowFont);
  logBox->setMaximumHeight(150);
  logBox->setFocusPolicy(Qt::NoFocus);
  logBox->setToolTip(tr("Napló"));
  logBox->setLineWrapMode(QTextEdit::NoWrap);
  userNameLabel->setContentsMargins(2,0,2,0);
  userNameLabel->setToolTip(tr("Felhasználó"));
  remoteDatabasePathLabel->setText(pth.pathes[PTH_remoteDatabase]);
  remoteDatabasePathLabel->setContentsMargins(2,0,2,0);
  remoteDatabasePathLabel->setToolTip(tr("Távoli adatbázis"));
  localDatabasePathLabel->setText(pth.pathes[PTH_localDatabase]);
  localDatabasePathLabel->setContentsMargins(2,0,2,0);
  localDatabasePathLabel->setToolTip(tr("Helyi adatbázis"));
  statusBar->setContentsMargins(2,0,2,0);
  statusBar->addPermanentWidget(localDatabasePathLabel);
  statusBar->addPermanentWidget(synchroniseStatus);
  statusBar->addPermanentWidget(remoteDatabasePathLabel);
  statusBar->addWidget(userNameLabel);
  synchroniseStatus->setFixedWidth(40);
  synchroniseStatus->setAlignment(Qt::AlignCenter);
  synchroniseStatus->setToolTip(tr("Kapcsolat / Automatikus frissítés"));

  checkRemoteDatabase();
  sumTableView = new SumTableView(this, layout);

  connect(sumTableView, SIGNAL(selectedYearChanged()),
          this, SLOT(toggleJumpThisDay()));
  connect(sumTableView, SIGNAL(archiveViewSelected()),
          this, SLOT(setMenuStatus()));
  connect(remoteAccessCheckTimer, SIGNAL(timeout()),
          this, SLOT(checkRemoteDatabase()));
  connect(synchroniseTimer, SIGNAL(timeout()),
          this, SLOT(synchroniseDatabases()));
  connect(settingsWindow, SIGNAL(textSaved()),
          this, SLOT(updateSettings()));
  connect(searchWindow, SIGNAL(searchValue(bool, QString, QString, QDateTime, QDateTime, bool)),
          this, SLOT(searchValue(bool, QString, QString, QDateTime, QDateTime, bool)));

  remoteAccessCheckTimer->start(CHECKREMOTEDBC);
  if (autoUpdateFlag) synchroniseTimer->start(SYNCHRONISETACT);

  sumTableView->showSumTableViewItems();
  this->setUpdatesEnabled(true);
  this->blockSignals(false);
  D_CONSTRUCT("")
}

MainWindow::~MainWindow()
{
  adminAccess = false; // always exit with admin mode off
  saveStatus();
  checkErrorInLogFile();

  delete searchWindow;
  delete userNameLabel;
  delete remoteDatabasePathLabel;
  delete statusBar;
  delete sumTableView;
  delete layout;
  delete mainWidget;
  delete splash;

  D_DESTRUCT("")
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
///////////////////////////////////////////////////////////////////////////////

void MainWindow::checkErrorInLogFile()
{
  FileAccess file(pth.pathes[FLE_log]);
  QByteArray ba;
  if (file.read())
  {
    file.get(ba);
    if (ba.contains("<font color=\"red\">"))
    {
      if (Msg::pshow(MSG_QUESTION, MSG_ERRORINLOG) == QMessageBox::Yes)
        mail->provideMailToSend(pth.pathes[FLE_log]);
    }
  }
}

//
// create complete path if not available or send error
//
void MainWindow::createPath(QString path)
{
  if (!QDir(path).exists())
  {
    QDir dir;
    if (!dir.mkdir(path))
    {
      Msg::log(MSG_ERROR, QString("Az útvonal nem hozható létre: %1").arg(path));
    }
  }
}

//
// set menu satus
// some menus must not be available when
// - no remote connection
// - no admin access
// - archive view is active
//
void MainWindow::setMenuStatus(void)
{
  if (sumTableView != NULL)
  {
    bool archiveNotActive = sumTableView->archiveList->currentIndex() == 0;
    bool viewCountGreaterZero = sumTableView->tableViews.count() > 0;
    jumpDateAct->setEnabled(viewCountGreaterZero);
    findValueAct->setEnabled(viewCountGreaterZero);
    entryListAct->setEnabled(viewCountGreaterZero);
    importToLocalDatabaseAct->setEnabled(archiveNotActive);
    clearLocalDatabasesAct->setEnabled(remote && archiveNotActive);
    editTableAsTextAct->setEnabled(remote && adminAccess && viewCountGreaterZero);
    generateNextYearDbcsAct->setEnabled(remote && adminAccess && viewCountGreaterZero);
    generateDbcAct->setEnabled(remote && adminAccess && viewCountGreaterZero);
    clearSelDatabaseAct->setEnabled(remote && adminAccess && viewCountGreaterZero);
    clearYearDatabaseAct->setEnabled(remote && adminAccess && viewCountGreaterZero);
    keyGenAct->setEnabled(adminAccess && viewCountGreaterZero);
    switchAdminAct->setChecked(adminAccess);
    synchroniseDatabasesAct->setEnabled(archiveNotActive && viewCountGreaterZero && remote);
  }
  else
  {
    synchroniseDatabasesAct->setEnabled(false);
    jumpDateAct->setEnabled(false);
    findValueAct->setEnabled(false);
    entryListAct->setEnabled(false);
  }
}


//
// check remote database access
//
void MainWindow::checkRemoteDatabase(void)
{
  synchroniseTimer->blockSignals(!isActiveWindow());
  if (QDir(pth.pathes[PTH_remoteDatabase]).isReadable())
  {
    remote = true;
    if (autoUpdateFlag)
      synchroniseTimer->start(SYNCHRONISETACT);
    else
      synchroniseStatus->setPixmap(PIXMAP("on.png"));
  }
  else
  {
    remote = false;
    if (autoUpdateFlag) synchroniseTimer->stop();
    synchroniseStatus->setPixmap(PIXMAP("off.png"));
  }
  setMenuStatus();
}

//
// toggle administrator access features
//
void MainWindow::toggleAdminAccess(void)
{
  if ((adminAccess) &&
      (adminKey != correctAdminKey))
  {
    bool isValueSet = false;
    userKeyInput(&adminKey, &isValueSet);
    if (isValueSet)
    {
      if (adminKey != correctAdminKey)
      {
        adminKey = "";
        adminAccess = false;
        Msg::log(MSG_ERROR, "Adminisztrátori kulcs beállítása sikertelen");
      }
      else
      {
        appSettings.setValue("key", adminKey);
        Msg::log(MSG_WARNING, "Adminisztrátori kulcs beállítása sikeres");
        if (sumTableView->yearSelect->currentText().isEmpty())
          sumTableView->loadNewYear(QString("%1").arg(QDate::currentDate().year()));
        else
          sumTableView->loadNewYear(sumTableView->yearSelect->currentText());
      }
    }
    else
    {
      adminAccess = false;
    }
  }
  if (adminAccess)
  {
    userNameLabel->setStyleSheet("background: yellow");
    Msg::log(MSG_WARNING, "adminisztrátor módba lépett");
  }
  else
  {
    if (adminKey == correctAdminKey) Msg::log(MSG_INFO, "kilépett az adminisztrátor módból");
    userNameLabel->setStyleSheet("");
  }
  setMenuStatus();
}

//
// user access key input returns false if initial key is unchanged
//
bool MainWindow::userKeyInput(QString* key, bool* isValueSet)
{
  SetKey* accessKey = new SetKey(this, key, isValueSet);
  QString initialKey(*key);
  accessKey->setWindowModality(Qt::WindowModal);
  QEventLoop* eventLoop = new QEventLoop(this);
  connect(accessKey, SIGNAL(destroyed()),
          eventLoop, SLOT(quit()));
  accessKey->show();
  eventLoop->exec();
  delete eventLoop;
  return (initialKey != *key);
}

void MainWindow::restoreStatus(void)
{
  // window and menu status
  QSettings settings("CashFlow", "SavedState");
  if (settings.value("windowState").isNull())
  { // no registry entry - default size
    setGeometry(desktop->frameGeometry().left() + (desktop->frameGeometry().width() - INITWIDTH) / 2,
                desktop->frameGeometry().top()  + (desktop->frameGeometry().height() - INITHEIGHT) / 2,
                INITWIDTH, INITHEIGHT);
  }
  else
  {
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
  }
  if ((desktop->width() < frameGeometry().left()+frameGeometry().width()) ||
      (frameGeometry().left() < 0))
    move(0,frameGeometry().top());
  if ((desktop->height() < frameGeometry().top()+frameGeometry().height()) ||
      (frameGeometry().top() < 0))
    move(frameGeometry().left(), 0);
  autoUpdateFlag = settings.value("flags/autoUpdate", "true").toBool();
  outputLogWarningsFlag = settings.value("flags/outputLogWarnings", "true").toBool();
  outputLogInfosFlag = settings.value("flags/outputLogInfos", "true").toBool();
  outputLogNotesFlag = settings.value("flags/outputLogNotes", "false").toBool();
  summaryVisible = settings.value("flags/summaryVisible", "true").toBool();
  // application settings
  adminKey              = appSettings.value("key", "00000000000000000000000000000000").toString();
}

void MainWindow::saveStatus(void)
{
  QSettings settings("CashFlow", "SavedState");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState(installedVersion.replace(".","").toInt()));
  settings.setValue("flags/autoUpdate", autoUpdateFlag);
  settings.setValue("flags/outputLogWarnings", outputLogWarningsFlag);
  settings.setValue("flags/outputLogInfos", outputLogInfosFlag);
  settings.setValue("flags/outputLogNotes", outputLogNotesFlag);
  settings.setValue("flags/summaryVisible", summaryVisible);
}

void MainWindow::createActions()
{
  //--------
  synchroniseDatabasesAct = new QAction(tr("Frissítés"), this);
  synchroniseDatabasesAct->setShortcut(QKeySequence("F"));
  synchroniseDatabasesAct->setStatusTip(tr("A kiválasztott év adatainak szinkronizálása a központi adatbázissal"));
  synchroniseDatabasesAct->setIcon(ICON("synchronise.png"));
  connect(synchroniseDatabasesAct, SIGNAL(triggered()), this, SLOT(forceSynchroniseDatabases()));

  openExcelProjectAct = new QAction(tr("Projekt tábla"), this);
  openExcelProjectAct->setShortcut(QKeySequence("P"));
  openExcelProjectAct->setStatusTip(tr("Excel projekt tábla megnyitása"));
  openExcelProjectAct->setIcon(ICON("excel.png"));
  connect(openExcelProjectAct, SIGNAL(triggered()), this, SLOT(openExcelProject()));

  importToLocalDatabaseAct = new QAction(tr("Import"), this);
  importToLocalDatabaseAct->setShortcut(QKeySequence("I"));
  importToLocalDatabaseAct->setStatusTip(tr("Importálás tabulátorokkal elválasztott szövegfájlból"));
  importToLocalDatabaseAct->setIcon(ICON("import.png"));
  connect(importToLocalDatabaseAct, SIGNAL(triggered()), this, SLOT(importToLocalDatabase()));

  exportFromLocalDatabaseAct = new QAction(tr("Export"), this);
  exportFromLocalDatabaseAct->setShortcut(QKeySequence("E"));
  exportFromLocalDatabaseAct->setStatusTip(tr("Exportálás tabulátorokkal elválasztott szövegfájlba"));
  exportFromLocalDatabaseAct->setIcon(ICON("export.png"));
  connect(exportFromLocalDatabaseAct, SIGNAL(triggered()), this, SLOT(exportFromLocalDatabase()));

  showLogAct = new QAction(tr("Napló"), this);
  showLogAct->setShortcut(QKeySequence("N"));
  showLogAct->setStatusTip(tr("A napló fájl megnyitása"));
  showLogAct->setIcon(ICON("record.png"));
  connect(showLogAct, SIGNAL(triggered()), this, SLOT(showLog()));

  clearLogAct = new QAction(tr("Napló törlése"), this);
  clearLogAct->setShortcut(QKeySequence("T"));
  clearLogAct->setStatusTip(tr("A napló ablak és a napló fájl törlése"));
  clearLogAct->setIcon(ICON("clearlog.png"));
  connect(clearLogAct, SIGNAL(triggered()), this, SLOT(clearLog()));

  clearLocalDatabasesAct = new QAction(tr("Helyi adatbázis törlése"), this);
  clearLocalDatabasesAct->setShortcut(QKeySequence("H"));
  clearLocalDatabasesAct->setStatusTip(tr("A kiválasztott helyi adatbázis fájl törlése - a legutolsó frissítés utáni adatok elvesznek"));
  clearLocalDatabasesAct->setIcon(ICON("clearlocaldbc.png"));
  connect(clearLocalDatabasesAct, SIGNAL(triggered()), this, SLOT(clearLocalDatabase()));

  exitApplicationAct = new QAction(tr("Kilépés"), this);
  exitApplicationAct->setShortcut(QKeySequence("Shift+L"));
  exitApplicationAct->setStatusTip(tr("Kilépés az alkalmazásból, a központi adatok frissítése nélkül"));
  exitApplicationAct->setIcon(ICON("exit.png"));
  connect(exitApplicationAct, SIGNAL(triggered()), this, SLOT(exitApplication()));
  //--------
  jumpDateAct = new QAction(tr("Ugrás a mai napra"), this);
  jumpDateAct->setShortcut(QKeySequence("M"));
  jumpDateAct->setStatusTip(tr("A kiválasztott sor mai dátumhoz tartozó oszlopának aktiválása"));
  connect(jumpDateAct, SIGNAL(triggered()), this, SLOT(jumpActDate()));

  findValueAct = new QAction(tr("Érték keresése"), this);
  findValueAct->setStatusTip(tr("Adat keresése a kiválasztott adatbázisban"));
  findValueAct->setShortcut(QKeySequence("K"));
  connect(findValueAct, SIGNAL(triggered()), this, SLOT(findValue()));

  entryListAct = new QAction(tr("Bejegyzések"), this);
  entryListAct->setStatusTip(tr("Bejegyzések listája a kiválasztott cellában"));
  entryListAct->setShortcut(QKeySequence("B"));
  connect(entryListAct, SIGNAL(triggered()), this, SLOT(showEntryList()));

  hideSummaryAct = new QAction(tr("Összegzõ"), this);
  hideSummaryAct->setStatusTip(tr("Az legfelsõ összegzõ táblázat láthatóságának váltása"));
  hideSummaryAct->setShortcut(QKeySequence("S"));
  hideSummaryAct->setCheckable(true);
  hideSummaryAct->setChecked(summaryVisible);
  connect(hideSummaryAct, SIGNAL(triggered()), this, SLOT(hideSummary()));

  logWindowWarningAct = new QAction(tr("Figyelmeztetés"), this);
  logWindowWarningAct->setStatusTip(tr("Adatbázis változást eredményezõ események mutatása a napló ablakban"));
  logWindowWarningAct->setCheckable(true);
  logWindowWarningAct->setChecked(outputLogWarningsFlag);
  connect(logWindowWarningAct, SIGNAL(triggered()), this, SLOT(logWindowWarning()));

  logWindowInfoAct = new QAction(tr("Információ"), this);
  logWindowInfoAct->setStatusTip(tr("Kevésbé fontos események mutatása a napló ablakban"));
  logWindowInfoAct->setCheckable(true);
  logWindowInfoAct->setChecked(outputLogInfosFlag);
  connect(logWindowInfoAct, SIGNAL(triggered()), this, SLOT(logWindowInfo()));

  logWindowNoteAct = new QAction(tr("Megjegyzés"), this);
  logWindowNoteAct->setStatusTip(tr("Hibakeresést segítõ naplóbejegyzések mutatása a napló ablakban"));
  logWindowNoteAct->setCheckable(true);
  logWindowNoteAct->setChecked(outputLogNotesFlag);
  connect(logWindowNoteAct, SIGNAL(triggered()), this, SLOT(logWindowNote()));
  //--------
  setDatabasePathAct = new QAction(tr("Adatbázis útvonala"), this);
  setDatabasePathAct->setStatusTip(tr("Adatbázis elérésének megadása - csak betûjelhez rendelt hálózati meghajtóval használható"));
  //setDatabasePathAct->setIcon(ICON("path.png"));
  connect(setDatabasePathAct, SIGNAL(triggered()), this, SLOT(setDatabasePath()));

  settingsEditAct = new QAction(tr("Adatbázis kulcs"), this);
  settingsEditAct->setStatusTip(tr("Hozzáférést biztosító adatbázis kulcs megadása - az adminisztrátortól kérhetõ"));
  //settingsEditAct->setIcon(ICON("lock.png"));
  connect(settingsEditAct, SIGNAL(triggered()), this, SLOT(addDatabaseAccessKey()));

  autoUpdateAct = new QAction(tr("Automatikus frissítés"), this);
  autoUpdateAct->setStatusTip(tr("Szinkronizálás a központi adatbázissal percenként"));
  autoUpdateAct->setCheckable(true);
  autoUpdateAct->setChecked(autoUpdateFlag);
  //autoUpdateAct->setIcon(ICON("robot.png"));
  connect(autoUpdateAct, SIGNAL(triggered()), this, SLOT(autoUpdateSlot()));

  appSettingsAct = new QAction(tr("Beállítások"), this);
  appSettingsAct->setStatusTip(tr("Az alkalmazás beállításai"));
  //appSettingsAct->setIcon(ICON("settings.png"));
  connect(appSettingsAct, SIGNAL(triggered()), this, SLOT(editSettings()));
  //--------
  switchAdminAct = new QAction(tr("Adminisztrátor mód"), this);
  switchAdminAct->setStatusTip(tr("Adminisztrátori üzemmód bekapcsolása - csak a kulcs birtokában lehetséges"));
  switchAdminAct->setCheckable(true);
  connect(switchAdminAct, SIGNAL(triggered()), this, SLOT(switchAdmin()));

  editTableAsTextAct = new QAction(tr("Javítás szövegként"), this);
  editTableAsTextAct->setStatusTip(tr("A kiválasztott központi adatbázis javítása szövegként - sor átnevezése, áthelyezése, törlése, cseréje"));
  connect(editTableAsTextAct, SIGNAL(triggered()), this, SLOT(editTableAsText()));

  keyGenAct = new QAction(tr("Kulcs Generálás"), this);
  keyGenAct->setStatusTip(tr("Adatfájlok hozzáférési kulcsainak generálása"));
  connect(keyGenAct, SIGNAL(triggered()), this, SLOT(generateKeys()));

  generateDbcAct = new QAction(tr("Új datbázis"), this);
  generateDbcAct->setStatusTip(tr("Új datbázis generálása a kiválasztott adatbázis soraival"));
  connect(generateDbcAct, SIGNAL(triggered()), this, SLOT(generateDbc()));

  generateNextYearDbcsAct = new QAction(tr("Következõ év adatbázisai"), this);
  generateNextYearDbcsAct->setStatusTip(tr("Üres adatbázisok generálása a kiválasztott év adatbázisaiból a következõ évre"));
  connect(generateNextYearDbcsAct, SIGNAL(triggered()), this, SLOT(generateNextYearDbcs()));

  clearSelDatabaseAct = new QAction(tr("Adatok törlése"), this);
  clearSelDatabaseAct->setStatusTip(tr("A kiválasztott központi adatbázis bejegyzéseinek törlése"));
  connect(clearSelDatabaseAct, SIGNAL(triggered()), this, SLOT(clearRemoteDatabaseData()));

  clearYearDatabaseAct = new QAction(tr("Központi adatbázis törlése"), this);
  clearYearDatabaseAct->setStatusTip(tr("A kiválasztott központi adatbázis fájl törlése"));
  connect(clearYearDatabaseAct, SIGNAL(triggered()), this, SLOT(clearRemoteDatabase()));
  //--------
  showHelpAct = new QAction(tr("Leírás"), this);
  showHelpAct->setShortcut(QKeySequence("L"));
  showHelpAct->setStatusTip(tr("Leírás megjelenítése"));
  connect(showHelpAct, SIGNAL(triggered()), this, SLOT(showHelp()));

  showVersionAct = new QAction(tr("Verzió"), this);
  showVersionAct->setShortcut(QKeySequence("V"));
  showVersionAct->setStatusTip(tr("Program verzió megjelenítése"));
  connect(showVersionAct, SIGNAL(triggered()), this, SLOT(showVersion()));

  showVersionAct = new QAction(tr("Verzió"), this);
  showVersionAct->setShortcut(QKeySequence("V"));
  showVersionAct->setStatusTip(tr("Program verzió megjelenítése"));
  connect(showVersionAct, SIGNAL(triggered()), this, SLOT(showVersion()));

  sendMailWithSettingsAct = new QAction(tr("Beállítások küldése"), this);
  sendMailWithSettingsAct->setStatusTip(tr("Beállítások küldése az adminisztrátor e-mail címére"));
  connect(sendMailWithSettingsAct, SIGNAL(triggered()), this, SLOT(sendMailWithSettings()));

  sendMailWithLogAct = new QAction(tr("Napló küldése"), this);
  sendMailWithLogAct->setStatusTip(tr("Napló küldése az adminisztrátor e-mail címére"));
  connect(sendMailWithLogAct, SIGNAL(triggered()), this, SLOT(sendMailWithLog()));
  //--------
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&Adatok"));
  fileMenu->addAction(synchroniseDatabasesAct);
  fileMenu->addSeparator();
  fileMenu->addAction(openExcelProjectAct);
  fileMenu->addAction(importToLocalDatabaseAct);
  fileMenu->addAction(exportFromLocalDatabaseAct);
  fileMenu->addSeparator();
  fileMenu->addAction(showLogAct);
  fileMenu->addAction(clearLogAct);
  fileMenu->addAction(clearLocalDatabasesAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitApplicationAct);

  viewMenu = menuBar()->addMenu(tr("&Nézet"));
  viewMenu->addAction(jumpDateAct);
  viewMenu->addAction(findValueAct);
  viewMenu->addAction(entryListAct);
  viewMenu->addAction(hideSummaryAct);
  logWindowSubMenu = viewMenu->addMenu(tr("Napló ablak"));
  logWindowSubMenu->addAction(logWindowWarningAct);
  logWindowSubMenu->addAction(logWindowInfoAct);
  logWindowSubMenu->addAction(logWindowNoteAct);

  settingsMenu = menuBar()->addMenu(tr("&Beállítások"));
  settingsMenu->addAction(setDatabasePathAct);
  settingsMenu->addAction(settingsEditAct);
  settingsMenu->addAction(autoUpdateAct);
  settingsMenu->addSeparator();
  settingsMenu->addAction(appSettingsAct);

  administratorMenu = menuBar()->addMenu(tr("A&dminisztrátor"));
  administratorMenu->addAction(switchAdminAct);
  administratorMenu->addSeparator();
  administratorMenu->addAction(keyGenAct);
  administratorMenu->addAction(generateDbcAct);
  administratorMenu->addAction(generateNextYearDbcsAct);
  administratorMenu->addAction(editTableAsTextAct);
  administratorMenu->addSeparator();
  administratorMenu->addAction(clearSelDatabaseAct);
  administratorMenu->addAction(clearYearDatabaseAct);

  helpMenu = menuBar()->addMenu(tr("&Segítség"));
  helpMenu->addAction(showHelpAct);
  helpMenu->addAction(showVersionAct);
  helpMenu->addSeparator();
  helpMenu->addAction(sendMailWithSettingsAct);
  helpMenu->addAction(sendMailWithLogAct);

}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE ACTIONS
///////////////////////////////////////////////////////////////////////////////

void MainWindow::generateKeys()
{
  this->repaint();
  for (int i=0; i<sumTableView->fileNames.count(); i++)
  {
    QString fileName(sumTableView->fileNames.at(i));
    QString name = QFileInfo(fileName).fileName();
    FileAccess file(fileName);
    Msg::log(MSG_WARNING, QString("%1\t%2").arg(file.key).arg(file.segmentName));
  }
}

void MainWindow::setDatabasePath()
{
  this->repaint();
  QFileDialog* fileDialog = new QFileDialog(this);
  fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog->setFileMode(QFileDialog::Directory);
  fileDialog->setOptions(QFileDialog::ShowDirsOnly);
  if (fileDialog->exec())
  {
    QString path = fileDialog->selectedFiles().at(0);
    //fileDialog->stackUnder(this);
    fileDialog->close();
    this->repaint();
    QStringList el = QDir(path).entryList();
    int datCount = 0;
    for (int p=0; p<el.count(); p++) {if (el[p].contains(QRegExp("\\d+\\.\\d+\\.[^\\.]+\\.dat"))) {datCount++;}}
    if (datCount > 0)
    {
      pth.pathes[PTH_remoteDatabase] = path;
      pth.pathes[PTH_archiveDatabase] = QString("%1/archive").arg(pth.pathes[PTH_remoteDatabase]);
      appSettings.setValue("DatabasePath", pth.pathes[PTH_remoteDatabase].replace("/","\\"));
      remoteDatabasePathLabel->setText(pth.pathes[PTH_remoteDatabase]);
      sumTableView->loadNewYear(QVariant(QDate::currentDate().year()).toString());
      Msg::log(MSG_INFO, tr("a kiválasztott %1 útvonalon %2 adatbázis található").arg(path).arg(datCount));
    }
    else
    {
      Msg::log(MSG_ERROR, tr("a kiválasztott \"%1\" útvonalon nem találhatók adatbázisok").arg(path));
    }
  }
  delete fileDialog;
}

void MainWindow::editSettings()
{
  this->repaint();
  FileAccess file(pth.pathes[FLE_settings]);
  if (!file.read())
  {
    if (!file.write())
      Msg::show(MSG_ERROR, MSG_FILE_SAVE, pth.pathes[FLE_settings]);
  }
  settingsWindow->setFile(pth.pathes[FLE_settings]);
  settingsWindow->setWindowTitle(pth.pathes[FLE_settings]);
  settingsWindow->setGeometry(QRect(mapToGlobal(QPoint((frameGeometry().width() - 600)/2,
                                                       (frameGeometry().height()- 500)/2)),
                                           QSize(600, 500)));
  settingsWindow->show();
  settingsWindow->activateWindow();
}

void MainWindow::forceSynchroniseDatabases()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  splash->message(tr("frissítés folyamatban"), true);
  nextSynchronisationInSecs = 0;
  synchroniseDatabases();
  splash->finish();
}

void MainWindow::synchroniseDatabases()
{
  if (nextSynchronisationInSecs > 0)
  {
    nextSynchronisationInSecs--;
    synchroniseStatus->setText(QString("> %1 >").arg(QString("0%1").arg(nextSynchronisationInSecs).right(3)));
  }
  else
  {
    nextSynchronisationInSecs = AUTOUPDATEPERIOD;
    synchroniseStatus->setText(QString("> %1 >").arg(AUTOUPDATEPERIOD));
    sumTableView->synchroniseTables();
    sumTableView->showSumTableViewItems();
  }
}

void MainWindow::clearLog()
{
  this->repaint();
  if (Msg::pshow(MSG_QUESTION, MSG_CLEARLOGGEDDATA) == QMessageBox::Yes)
  {
    this->repaint();
    if (Msg::clearLog())
      Msg::log(MSG_WARNING, QString("%1 napló fájl törölve").arg(pth.pathes[FLE_log]));
    else
      Msg::log(MSG_WARNING, QString("%1 napló fájl törlése sikertelen").arg(pth.pathes[FLE_log]));
  }
}

void MainWindow::clearLocalDatabase()
{
  this->repaint();
  if (Msg::pshow(MSG_QUESTION, MSG_CLEARLOCALDATAFILE) == QMessageBox::Yes)
  {
    this->repaint();
    if (sumTableView->removeDatabaseFile(false))
      Msg::log(MSG_WARNING, QString("%1 adatbázis fájl törölve").arg(sumTableView->activeTableView->localFile));
    else
      Msg::log(MSG_WARNING, QString("%1 adatbázis fájl törlése sikertelen").arg(sumTableView->activeTableView->localFile));
    sumTableView->updateSummaryView();
  }
}

void MainWindow::importToLocalDatabase()
{
  this->repaint();
  if (!settings.contains("import"))
  {
    Msg::log(MSG_ERROR, "a beállítások között nem található a sorok összefüggését megadó \"import\" elem");
  }
  else if  (!settings["import"].contains("column"))
  {
    Msg::log(MSG_ERROR, "az import beállítások között nem található a kezdõ oszlopot megadó \"column\" érték");
  }
  else
  {
    synchroniseTimer->blockSignals(true);
    QFileDialog* fileDialog = new QFileDialog(this);
    fileDialog->setNameFilter(tr("Szövegfájl (*.txt *.csv)"));
    fileDialog->setDefaultSuffix("txt");
    fileDialog->setDirectory(pth.pathes[PTH_importExport]);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    if (fileDialog->exec())
    {
      QString fileName = fileDialog->selectedFiles().at(0);;
      fileDialog->close();
      this->repaint();
      FileAccess file(fileName);
      QByteArray fileText;
      if (file.read())
      {
        file.get(fileText);
        fileText.replace("\r", "");
        while (fileText.endsWith('\n')) {fileText.chop(1);}
        QList<QByteArray> rows = fileText.split('\n');
        stringMatrix* data = &sumTableView->activeTableView->tableModel->tableData->data;
        stringArray* rowHeader = &sumTableView->activeTableView->tableModel->tableData->rowHeader;
        QHash<QString, int> rowPositionForName;
        for (int r=0; r<rowHeader->count(); r++) {rowPositionForName[rowHeader->at(r)] = r;}
        sumTableView->activeTableView->tableModel->tableData->blockSignals(true);
        QHashIterator<QString, QString> row(settings["import"]);
        while (row.hasNext())
        {
          row.next();
          int dataRow = row.value().toInt();
          QString header = row.key();
          if (header != "column")
          {
            if (dataRow < rows.count())
            {
              int updatedCount = 0;
              QList<QByteArray> columns = rows[dataRow].split('\t');
              if (rowPositionForName.contains(header))
              {
                int tableRow = rowPositionForName.value(header);
                int dataColumn = settings["import"]["column"].toInt();
                int tableColumn = 0;
                while ((dataColumn<data->at(tableRow).count()) && (dataColumn<columns.count()) && (dataColumn>0))
                {
                  QString d = QVariant(columns[dataColumn]).toString();
                  if ((!d.isEmpty()) &&
                      (data->at(tableRow).at(tableColumn).toLongLong() != d.toLongLong()))
                  {
                    sumTableView->activeTableView->tableModel->tableData->addEntryLog(tableRow, tableColumn, d);
                    updatedCount++;
                  }
                  dataColumn++;
                  tableColumn++;
                }
                Msg::log(MSG_WARNING, tr("%1 adat importálva a(z) \"%2\" sorba").arg(updatedCount).arg(rowHeader->at(tableRow)));
              }
              else
              {
                Msg::log(MSG_ERROR, tr("az adatbázisnak nincs \"%1\" nevu sora").arg(header));
              }
            }
            else
            {
              Msg::log(MSG_ERROR, tr("az importálandó fájlnak nincs %1 sora").arg(dataRow));
            }
          }
        }
        sumTableView->activeTableView->tableModel->tableData->saveData();
        sumTableView->sumTableModel->sumTableData->updateSummedValuesAndRecalculate(sumTableView->activeTableView->tableModel->tableData);
        sumTableView->activeTableView->updateTableView();
        sumTableView->activeTableView->tableModel->tableData->blockSignals(false);
      }
    }
  }
  synchroniseTimer->blockSignals(false);
}

void MainWindow::exportFromLocalDatabase()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  QFileDialog* fileDialog = new QFileDialog(this);
  fileDialog->setNameFilter(tr("Szövegfájl (*.txt *.csv)"));
  fileDialog->setDefaultSuffix("txt");
  fileDialog->setDirectory(pth.pathes[PTH_importExport]);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->setFileMode(QFileDialog::AnyFile);
  fileDialog->selectFile(QFileInfo(QString(sumTableView->activeTableView->localFile)).fileName().replace(".dat",".txt"));
  if (fileDialog->exec())
  {
    QString fileName = fileDialog->selectedFiles().at(0);;
    fileDialog->close();
    this->repaint();
    FileAccess file(fileName);
    QByteArray fileText;
    stringMatrix* data = &sumTableView->activeTableView->tableModel->tableData->data;
    stringArray* rowHeader = &sumTableView->activeTableView->tableModel->tableData->rowHeader;
    stringArray* columnHeader = &sumTableView->activeTableView->tableModel->tableData->columnHeader;
    fileText.append("\t");
    for (int c=0; c<data->at(0).count(); c++)
    {
      fileText.append(columnHeader->at(c));
      fileText.append("\t");
    }
    fileText.append("\r\n");
    for (int r=0; r<data->count(); r++)
    {
      fileText.append(rowHeader->at(r));
      fileText.append("\t");
      for (int c=0; c<data->at(r).count(); c++)
      {
          fileText.append(data->at(r).at(c));
          fileText.append("\t");
      }
      fileText.append("\r\n");
    }
    file.put(fileText);
    file.write();
  }
  synchroniseTimer->blockSignals(false);
}

void MainWindow::clearRemoteDatabase()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  if (Msg::show(MSG_QUESTION, MSG_CLEARREMOTEDATAFILE, sumTableView->activeTableView->remoteFile) == QMessageBox::Yes)
  {
    this->repaint();
    splash->message(tr("adatbázis fájl\r\ntörlése folyamatban"), true);
    if (sumTableView->removeDatabaseFile(true))
    {
      Msg::log(MSG_WARNING, QString("%1 adatbázis fájl törölve").arg(sumTableView->activeTableView->remoteFile));
      sumTableView->loadNewYear(sumTableView->yearSelect->currentText());
    }
    else
      Msg::log(MSG_WARNING, QString("%1 adatbázis fájl törlése sikertelen").arg(sumTableView->activeTableView->remoteFile));
    splash->finish();
  }
  synchroniseTimer->blockSignals(false);
}

void MainWindow::clearRemoteDatabaseData()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  if (Msg::show(MSG_QUESTION, MSG_CLEARREMOTEDATA, sumTableView->activeTableView->remoteFile) == QMessageBox::Yes)
  {
    splash->message(tr("adatbázis fájl\r\nkiürítése folyamatban"), true);
    sumTableView->clearDatabase(true);
    sumTableView->updateSummaryView();
    sumTableView->activeTableView->updateGeometry();
    splash->finish();
  }
  synchroniseTimer->blockSignals(false);
}

void MainWindow::generateDbc()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  QFileDialog* fileDialog = new QFileDialog(this);
  fileDialog->setNameFilter(tr("Adatbázis (*.dat)"));
  fileDialog->setDefaultSuffix("dat");
  fileDialog->setDirectory(pth.pathes[PTH_remoteDatabase]);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->setFileMode(QFileDialog::AnyFile);
  QString newFile = QString("%1/%2.%3.uj_uzletag.dat").
                    arg(pth.pathes[PTH_remoteDatabase]).
                    arg(sumTableView->yearSelect->currentText()).
                    arg(QString("00%1").
                    arg(sumTableView->tableViews.count()+1).right(2));
  fileDialog->selectFile(newFile);
  if (fileDialog->exec())
  {
    newFile = fileDialog->selectedFiles().at(0);
    fileDialog->close();
    this->repaint();
    sumTableView->activeTableView->tableModel->tableData->localFile = newFile; // local <-- remote
    sumTableView->activeTableView->tableModel->tableData->eraseData();
    sumTableView->activeTableView->tableModel->tableData->saveData();
    sumTableView->activeTableView->tableModel->tableData->localFile =
      sumTableView->activeTableView->tableModel->localFile;
    sumTableView->loadNewYear(sumTableView->yearSelect->currentText());
  }
  synchroniseTimer->blockSignals(false);
}

void MainWindow::generateNextYearDbcs()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  int cyear = sumTableView->yearSelect->currentText().toInt();
  int nyear = sumTableView->yearSelect->itemText(sumTableView->yearSelect->count()-1).toInt() + 1;
  if (nyear == 1) nyear = QDate::currentDate().year();
  splash->message(tr("%1 év adatbázisainak\r\ngenerálása folyamatban").arg(nyear), true);
  if (sumTableView->fileNames.count() < 1)
  {
    QString newFile = QString("%1/%2.01.uzletag.dat").arg(pth.pathes[PTH_remoteDatabase]).arg(nyear);
    sumTableView->fileNames.append(newFile);
    FileAccess f(newFile);
    QByteArray ba;
    ba.append("(+) tétel 1");
    for (int d=0; d<QDate::currentDate().daysInYear(); d++) ba.append('\000');
    ba.append('\001');
    ba.append("(-) tétel 2");
    for (int d=0; d<QDate::currentDate().daysInYear(); d++) ba.append('\000');
    ba.append('\001');
    f.put(ba);
    f.encode();
    f.write();
    sumTableView->yearSelect->addItem(QVariant(nyear).toString());
  }
  else
  {
    bool added = false;
    QString cyearPattern = QString("/%1.").arg(cyear);
    QString nyearPattern = QString("/%1.").arg(nyear);
    for (int f = 0; f < sumTableView->fileNames.count(); f++)
    {
      if (sumTableView->fileNames[f].contains(cyearPattern))
      {
        QString newFile = sumTableView->fileNames[f];
        newFile.replace(cyearPattern, nyearPattern);
        newFile.replace(pth.pathes[PTH_localDatabase], pth.pathes[PTH_remoteDatabase]);
        if (QFile::copy(sumTableView->fileNames[f], newFile))
        { // file not yet exists
          FileAccess file(newFile, this);
          file.read();
          file.decode();
          QByteArray ba;
          file.get(ba);
          if (ba[ba.count()-2] == '~')
          { // remove version number
            ba.remove(ba.count()-2, 2);
          }
          ba.replace('\0',"\t");
          ba.replace('\1',"\n");
          QString st = QVariant(ba).toString();
          st.replace(QRegExp("\\t[0-9]+\\t"), "\t\t");
          st.replace(QRegExp("\\t\\[[^\\]]+\\]"), "");
          ba = QVariant(st).toByteArray();
          QByteArray n(1, '\0');
          QByteArray o(1, '\1');
          ba.replace('\t', n);
          ba.replace('\n', o);
          ba.append('~');
          ba.append('\0');
          file.put(ba);
          file.encode();
          if (file.write())
          {
            added = true;
          }
        }
      }
    }
    if (added)
    {
      sumTableView->yearSelect->addItem(QVariant(nyear).toString());
      Msg::log(MSG_WARNING, tr("%1 évi üres adatbázisok el\365állítva").arg(nyear));
    }
  }
  splash->finish();
  synchroniseTimer->blockSignals(false);
}

void MainWindow::switchAdmin()
{
  adminAccess = !adminAccess;
  toggleAdminAccess();
}

void MainWindow::exitApplication()
{
  adminAccess = false; // always exit with admin mode off
  QCoreApplication::quit();
}

void MainWindow::jumpActDate(void)
{ // ToDo: set all active columns together - why SS not working here?
  sumTableView->activeTableView->setActiveColumn(QDate::currentDate().dayOfYear()-1);
}

void MainWindow::showHelp(void)
{
  this->repaint();
  if (QFileInfo(pth.pathes[FLE_help]).exists())
  {
    splash->message("leírás\r\nmegnyitása folyamatban", true);
    QDesktopServices::openUrl(QUrl(pth.pathes[FLE_help].prepend( "file:///" )));
    splash->finish();
  }
  else
  {
    Msg::log(MSG_ERROR, tr("%1 nem elérhetõ, vagy már meg van nyitva").arg(pth.pathes[FLE_help]));
  }
}

void MainWindow::showVersion(void)
{
  splash->message(QString("<b><font color=black>CashFlow %1</font><br><br><small><small>Cserhalmi György 2o12</small></small></b>").arg(installedVersion), true);
  splash->finish();
}

void MainWindow::sendMailWithSettings(void)
{
  mail->provideMailToSend(pth.pathes[FLE_settings]);
}

void MainWindow::sendMailWithLog(void)
{
  mail->provideMailToSend(pth.pathes[FLE_log]);
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
///////////////////////////////////////////////////////////////////////////////

void MainWindow::logWindowWarning()
{
  outputLogWarningsFlag = logWindowWarningAct->isChecked();
}

void MainWindow::logWindowInfo()
{
  outputLogInfosFlag = logWindowInfoAct->isChecked();
}

void MainWindow::logWindowNote()
{
  outputLogNotesFlag = logWindowNoteAct->isChecked();
}

void MainWindow::autoUpdateSlot()
{
  if (autoUpdateAct->isChecked())
  {
    autoUpdateFlag = true;
    synchroniseTimer->start(SYNCHRONISETACT);
  }
  else
  {
    autoUpdateFlag = false;
    synchroniseTimer->stop();
  }
}

void MainWindow::findValue()
{
  if (sumTableView->getTableCount() > 0)
  {
    searchWindow->show();
    searchWindow->activateWindow();
  }
}

void MainWindow::showEntryList()
{
  if (sumTableView->getTableCount() > 0)
  {
    sumTableView->activeTableView->showEntryList();
  }
}

void MainWindow::hideSummary()
{
  if (sumTableView->getTableCount() > 0)
  {
    if (sumTableView->isVisible())
    {
      sumTableView->setVisible(false);
    }
    else
    {
      sumTableView->setVisible(true);
    }
  }
  summaryVisible = sumTableView->isVisible();
}

void MainWindow::editTableAsText(void)
{
  if (sumTableView->getTableCount() > 0)
  {
    sumTableView->activeTableView->editTableAsText();
  }
}

void MainWindow::toggleJumpThisDay(void)
{
  jumpDateAct->setEnabled(sumTableView->actualYear == QDate::currentDate().year());
}

void MainWindow::updateSettings()
{
  settings.clear();
  Settings::getSettingsFile(pth.pathes[FLE_settings]);
  Settings::setSettings();
  sumTableView->updateUserSettings();
  nextSynchronisationInSecs = AUTOUPDATEPERIOD;
  remoteAccessCheckTimer->start(CHECKREMOTEDBC);
}

void MainWindow::addDatabaseAccessKey(void)
{
  this->repaint();
  bool isValueSet = false;
  QString key = "00000000000000000000000000000000";
  userKeyInput(&key, &isValueSet);
  this->repaint();
  if (isValueSet)
  {
    for (int f=0; f<sumTableView->availableFileNames.count(); f++)
    {
      FileAccess file(sumTableView->availableFileNames.at(f));
      if (file.key == key)
      {
        Msg::log(MSG_WARNING, tr("%1 adatbázis elérhetõ").arg(file.pathName));
        appSettings.setValue(tr("keys/%1").arg(file.segmentName), key);
        sumTableView->loadNewYear(QString("%1").arg(QDate::currentDate().year()));
      }
    }
  }
}

void MainWindow::openExcelProject()
{
  this->repaint();
  synchroniseTimer->blockSignals(true);
  QFileDialog* fileDialog = new QFileDialog(this);
  fileDialog->setNameFilter(tr("Excel (*.xlsm)"));
  fileDialog->setDefaultSuffix("xlsm");
  fileDialog->setDirectory(pth.pathes[PTH_localDatabase]);
  fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog->setFileMode(QFileDialog::AnyFile);
  if (fileDialog->exec())
  {
    QString projectTableFileName = fileDialog->selectedFiles().at(0);
    fileDialog->close();
    this->repaint();
    if (QFileInfo(projectTableFileName).exists())
    {
      splash->message("projekt tábla\r\nmegnyitása folyamatban", true);
      QDesktopServices::openUrl(QUrl(projectTableFileName.prepend( "file:///" )));
      splash->finish();
    }
    else
    {
      Msg::log(MSG_ERROR, tr("%1 nem elérhetõ, vagy már meg van nyitva").arg(projectTableFileName));
    }
  }
  synchroniseTimer->blockSignals(false);
}

void MainWindow::showLog()
{
  logWindow->setFile(pth.pathes[FLE_log]);
  logWindow->setWindowTitle(pth.pathes[FLE_log]);
  logWindow->setGeometry(QRect(mapToGlobal(QPoint((frameGeometry().width() - 1000)/2,
                                                  (frameGeometry().height()- 500)/2)),
                                           QSize(1000, 500)));
  logWindow->show();
  logWindow->activateWindow();
}

void MainWindow::searchValue(bool forward, QString value, QString user, QDateTime fromdate, QDateTime todate, bool allpages)
{
  int step = 0;
  int r = 0;
  int c = 0;
  sumTableView->activeTableView->searchValue(forward, value, user, fromdate, todate, step, r, c);
  if (step)
  {
    sumTableView->activeTableView->gotoValue(r, c);
  }
  else if (allpages)
  {
    int id = sumTableView->activeTableId;
    do
    {
      if (forward)
      {
        if (id < sumTableView->tableViews.count() - 1)
        {
          id++;
          QModelIndex index = sumTableView->tableViews.at(id)->tableModel->index(0, 0);
          sumTableView->tableViews.at(id)->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
      }
      else
      {
        if (id > 0)
        {
          id--;
          QModelIndex index = sumTableView->tableViews.at(id)->tableModel->index(
            sumTableView->tableViews.at(id)->tableModel->tableData->rows-1,
            sumTableView->tableViews.at(id)->tableModel->tableData->columns-1);
          sumTableView->tableViews.at(id)->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
      }
      sumTableView->tableViews.at(id)->searchValue(forward, value, user, fromdate, todate, step, r, c);
      qDebug() << step << id << r << c;
    }
    while ((step == 0) &&
           (id < sumTableView->tableViews.count() - 1) &&
           (id > 0));
    if (step)
    {
      sumTableView->activateTable(id);
      QModelIndex index = sumTableView->tableViews.at(id)->tableModel->index(r, c);
      sumTableView->tableViews.at(id)->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    }
  }
}

void MainWindow::activateTableView()
{
  activateWindow();
  setFocus();
  sumTableView->activeTableView->setFocus();
}
