#ifndef MAIN_H
#define MAIN_H

#include "splashscreen.h"
#include "textedit.h"
#include "pathes.h"
#include <QString>
#include <QSettings>
#include <QDesktopWidget>
#include <QTextBrowser>
#include <QStringList>

#define MAXTABLEROWS          50
#define INITWIDTH            800
#define INITHEIGHT           600
#define SYNCHRONISETACT     1000
#define TROWHEIGHT          tableRowHeight
#define TCOLUMWIDHT         tableColumnWidth
#define SROWHEIGHT          tableRowHeight
#define SCOLUMWIDHT         sumColumnWidth
#define VHEADERWIDTH        tableHeaderWidth
#define AUTOUPDATEPERIOD    autoRefresh
#define CHECKREMOTEDBC      checkRemote

#define ICON(name)     QIcon(QString(":/icons/%2").arg(""name""))
#define PIXMAP(name)   QPixmap(QString(":/icons/%2").arg(""name""))

#define GETSILENT      bool __bs = signalsBlocked(); \
                       blockSignals(true);

#define GETLOUD        blockSignals(__bs);

extern QSettings       appSettings;
extern Pathes          pth;
extern SplashScreen*   splash;
extern QDesktopWidget* desktop;
extern QTextBrowser*   logBox;
extern QString         userName;
extern QString         installedVersion;
extern QString         adminKey;
extern QString         correctAdminKey;

extern QFont           tableHeaderFont;
extern QFont           logWindowFont;
extern QFont           tableCellFont;
extern QFont           textEditFont;
extern QFont           calendarFont;
extern QFont           comboBoxFont;

extern QColor          cursorActiveColor;
extern QString         cursorActiveColorString;
extern QColor          cursorInactiveColor;
extern QString         cursorInactiveColorString;
extern QColor          weekEndColor;
extern QColor          archiveBackgroundColor;

extern bool            adminAccess;
extern bool            remote;
extern bool            autoUpdateFlag;
extern bool            outputLogWarningsFlag;
extern bool            outputLogInfosFlag;
extern bool            outputLogNotesFlag;
extern bool            summaryVisible;
extern bool            fileLoggingSuppressed;

extern int             tableRowHeight;
extern int             tableColumnWidth;
extern int             sumColumnWidth;
extern int             tableHeaderWidth;
extern int             checkRemote;
extern int             autoRefresh;

extern TextEdit*       settingsWindow;
extern TextEdit*       logWindow;
extern TextEdit*       tableEditWindow;

extern QStringList getFileList(QString folder, QString filter, bool recursive);

#endif // MAIN_H
