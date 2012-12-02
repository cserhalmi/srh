!include "MUI.nsh"
!include "Sections.nsh"
!include "FileFunc.nsh"
!insertmacro Locate
 
Var /GLOBAL switch_overwrite
!include "MoveFileFolder.nsh"

;--------------------------------
; Configuration
 
  Name "CashFlow"
  OutFile "cashflow_v1.1.5.exe"
  AllowRootDirInstall true
  ShowInstDetails show
  ShowUnInstDetails show
  SetOverwrite on
  Var /GLOBAL appDir
  Var /GLOBAL appDirBaseName
  Var /GLOBAL dataDir
  Var /GLOBAL adminKey
  Var /GLOBAL instVersion
  Var /GLOBAL adminKeyRef
  Var /GLOBAL version
  Var /GLOBAL qtversion
  
!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

;--------------------------------
; Modern UI Configuration
 
  !define MUI_INSTFILESPAGE_COLORS "000000 FFFFFF" ;Two colors
 
  !define MUI_COMPONENTSPAGE_NODESC
  !define MUI_ABORTWARNING
 
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "..\icons\wizard.bmp"
  !define MUI_ICON "..\icons\logo.ico"
  !define MUI_UNICON "..\icons\unlogo.ico"
  
  !define MUI_WELCOMEPAGE
  !insertmacro MUI_PAGE_WELCOME
  
  !define MUI_COMPONENTSPAGE
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW compPageShow
  !insertmacro MUI_PAGE_COMPONENTS
  
  Page Custom AdminKeyCustomPageFunction AdminKeyCustomPageLeave
  
  !define MUI_PAGE_CUSTOMFUNCTION_PRE appDirPre
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW appDirShow
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE appDirLeave
  !insertmacro MUI_PAGE_DIRECTORY
  
  !define MUI_PAGE_CUSTOMFUNCTION_PRE dataDirPre
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW dataDirShow
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE dataDirLeave
  !insertmacro MUI_PAGE_DIRECTORY
  
  !insertmacro MUI_PAGE_INSTFILES
 
  !insertmacro MUI_LANGUAGE "Hungarian"
  
;--------------------------------
; Installer Types
 
  InstType "Szok�sos"
  InstType "Teljes"

;--------------------------------
; Installer Sections
 
Function .onInit
  SetShellVarContext current
  StrCpy $version "1.1.5"
  StrCpy $qtversion "4.8.3"
  StrCpy $switch_overwrite 0
  ReadRegStr $0 HKCU "Software\CashFlow\ApplicationSettings" "DatabasePath"
  ReadRegStr $1 HKCU "Software\CashFlow\ApplicationSettings" "ApplicationPath"
  ReadRegStr $2 HKCU "Software\CashFlow\ApplicationSettings" "InstalledVersion"
  ReadRegStr $3 HKCU "Software\CashFlow\ApplicationSettings" "key"
  ${If} $0 == ""
    StrCpy $0 "K:\CashFlow\database"
  ${EndIf}
  ${If} $1 == ""
    StrCpy $1 "C:\Program Files\CashFlow"
  ${EndIf}
  ${If} $2 == ""
    StrCpy $2 "1.0.0"
  ${EndIf}
  ${If} $3 == ""
    StrCpy $3 "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
  ${EndIf}
  StrCpy $dataDir $0
  StrCpy $appDir $1
  StrCpy $instVersion $2
  StrCpy $adminKey $3
  StrCpy $adminKeyRef "AE3FF9435120484DB4BA03C17E02FD8E"
FunctionEnd
 
Section "Le�r�s"
  SectionIn 1 2 3
  SetOutPath $appDir\help
  File ..\help\index.htm
  SetOutPath $appDir\help\index_files
  File ..\help\index_files\*.*
  SetOutPath $appDir
SectionEnd

Section "Alkalmaz�s"
  SectionIn 1 2 3
  SetOutPath $appDir
  File C:\QtSDK\Desktop\Qt\4.8.3\bin\QtGui4.dll
  File C:\QtSDK\Desktop\Qt\4.8.3\bin\QtNetwork4.dll
  File C:\QtSDK\Desktop\Qt\4.8.3\bin\QtCore4.dll
  File C:\QtSDK\Desktop\Qt\4.8.3\bin\libgcc_s_dw2-1.dll
  File C:\QtSDK\Desktop\Qt\4.8.3\bin\mingwm10.dll
  File srh.exe
  SetOutPath $appDir\icons
  File ..\icons\logo.ico
  File ..\icons\unlogo.ico
  SetOutPath $APPDATA\CashFlow
  File "..\settings_temp.txt"
  SetOutPath $APPDATA\CashFlow\database
  SetOverwrite ifnewer
  File ..\database\*.xlsm
  SetOverwrite on
  SetOutPath $appDir
  
  ; working directory changed from 1.1.4 to 1.1.5
  ${If} ${FileExists} `$appDir\database\*.dat`
    !insertmacro MoveFolder "$appDir\database\" "$APPDATA\CashFlow\database\" "*.dat"
  ${EndIf}
  ${If} ${FileExists} `$appDir\database\*.xlsm`
    !insertmacro MoveFolder "$appDir\database\" "$APPDATA\CashFlow\database\" "*.xlsm"
  ${EndIf}
  ${If} ${FileExists} `$appDir\*.txt`
    !insertmacro MoveFolder "$appDir\" "$APPDATA\CashFlow\" "*.txt"
  ${EndIf}
  ${If} ${FileExists} `$appDir\export\*.*`
    !insertmacro MoveFolder "$appDir\export\" "$APPDATA\CashFlow\export\" "*.*"
  ${EndIf}
 
  WriteRegStr HKCU "Software\CashFlow\ApplicationSettings" "ApplicationPath" $appDir
  WriteRegStr HKCU "Software\CashFlow\ApplicationSettings" "DatabasePath" $dataDir
  WriteRegStr HKCU "Software\CashFlow\ApplicationSettings" "WorkingPath" "$APPDATA\CashFlow"
  WriteRegStr HKCU "Software\CashFlow\ApplicationSettings" "InstalledVersion" $version
  
  WriteUninstaller "$appDir\uninstall.exe"
  
SectionEnd
 
SubSection /E "Hivatkoz�sok"
  Section "Asztal" desktopIconSection
    SectionIn 1 2 3
    Delete "$DESKTOP\CashFlow.lnk"
    CreateShortCut "$DESKTOP\CashFlow.lnk" "$appDir\srh.exe" "" "$appDir\icons\logo.ico" 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F1 "CashFlow tervez�s"
  SectionEnd
  Section "Ind�t� Men�" startupIconSection
    SectionIn 1 2 3
    SetShellVarContext all
    Delete "$SMPROGRAMS\CashFlow\CashFlow.lnk"
    Delete "$SMPROGRAMS\CashFlow\Projektek.lnk"
    Delete "$SMPROGRAMS\CashFlow\Uninstall.lnk"
    SetShellVarContext current
    Delete "$SMPROGRAMS\CashFlow\CashFlow.lnk"
    Delete "$SMPROGRAMS\CashFlow\Projektek.lnk"
    Delete "$SMPROGRAMS\CashFlow\Uninstall.lnk"
    CreateDirectory "$SMPROGRAMS\CashFlow"
    CreateShortCut "$SMPROGRAMS\CashFlow\CashFlow.lnk" "$appDir\srh.exe" "" "$appDir\icons\logo.ico" 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F1 "CashFlow tervez�s"
    CreateShortCut "$SMPROGRAMS\CashFlow\Uninstall.lnk" "$appDir\Uninstall.exe" "" "$appDir\icons\unlogo.ico" 0 SW_SHOWNORMAL
  SectionEnd
SubSectionEnd
 
Section "Adatb�zis" databaseSection
    SectionIn 2
    SetOutPath $dataDir
    SetOverwrite off
    File ..\..\srh\database\*.dat
    SetOverwrite on
SectionEnd

;--------------------------------
; Uninstaller Sections
 
function un.onInit
  MessageBox MB_OKCANCEL "Elt�vol�tja a CashFlow alkalmaz�st?" IDOK next
    Abort
  next:
  !insertmacro VerifyUserIsAdmin
functionEnd
 
section "uninstall"
  SetShellVarContext all
  RMDir /r /REBOOTOK $INSTDIR 
  RMDir /r /REBOOTOK "$APPDATA\CashFlow"
  Delete "$DESKTOP\CashFlow.lnk"
  Delete "$SMPROGRAMS\CashFlow\CashFlow.lnk"
  Delete "$SMPROGRAMS\CashFlow\Projektek.lnk"
  Delete "$SMPROGRAMS\CashFlow\Uninstall.lnk"
  SetShellVarContext current
  RMDir /r /REBOOTOK $INSTDIR 
  RMDir /r /REBOOTOK "$APPDATA\CashFlow"
  Delete "$DESKTOP\CashFlow.lnk"
  Delete "$SMPROGRAMS\CashFlow\CashFlow.lnk"
  Delete "$SMPROGRAMS\CashFlow\Projektek.lnk"
  Delete "$SMPROGRAMS\CashFlow\Uninstall.lnk"
  DeleteRegKey HKCU "Software\CashFlow"
sectionEnd

;--------------------------------
; Custom Page Functions

Function AdminKeyCustomPageFunction
  ${If} $adminKey != $adminKeyRef 
    SectionGetFlags ${databaseSection} $0
    ${If} $0 > 0
      ReserveFile "adminkey.ini"
      !insertmacro MUI_HEADER_TEXT "Adminisztr�tori Kulcs" "Ha �n adminisztr�tor, adja meg a kulcsot"
      !insertmacro MUI_INSTALLOPTIONS_EXTRACT "adminkey.ini"
      !insertmacro MUI_INSTALLOPTIONS_DISPLAY "adminkey.ini"
    ${EndIf}
  ${EndIf}
FunctionEnd

Function AdminKeyCustomPageLeave
  Push $R0
  Push $R1
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "adminkey.ini" "Field 1" "HWND"
  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "adminkey.ini" "Field 1" "State"
  ${If} $R1 == $adminKeyRef
    StrCpy $adminKey $R1
  ${EndIf}
  ${If} $adminKey != $adminKeyRef
    MessageBox MB_OK "Hib�s adminisztr�tori kulcsot adott meg.$\n$\rAz adatb�zis nem telep�thet�."
    SectionSetFlags ${databaseSection} 0
  ${Else}
    WriteRegStr HKCU "Software\CashFlow\ApplicationSettings" "key" $R1
  ${EndIf}
  Pop $R1
  Pop $R0
FunctionEnd

;--------------------------------
;Installer Functions

Function compPageShow
  !insertmacro MUI_HEADER_TEXT "CashFlow �sszetv�inek kiv�laszt�sa" "V�lassza ki, hogy a CashFlow alkalmaz�s mely r�sz�t telep�ti, vagy friss�ti!"
  !insertmacro MUI_INNERDIALOG_TEXT 1006 "Jel�lje be azokat az �sszetev�ket, amelyeket telep�teni k�v�n. Kattintson a Tov�bb-ra a folytat�shoz. 'Adatb�zis' telep�t�shez k�sz�tse el� az adminisztrtr�tori kulcsot."
FunctionEnd
  
Function appDirPre
  StrCpy $INSTDIR $appDir
FunctionEnd
  
Function dataDirPre
  StrCpy $INSTDIR $dataDir
  IfFileExists $INSTDIR\*.dat 0 +2
  Abort
FunctionEnd

Function appDirShow
  !insertmacro MUI_HEADER_TEXT "Telep�t�s helye" "V�lassza ki, hogy a $(^Name) alkalmaz�st melyik mapp�ba telep�ti!"
  !insertmacro MUI_INNERDIALOG_TEXT 1006 "A CashFlow alkalmaz�s a k�vetkez� mapp�ba ker�l.$\r$\n$\r$\nM�sik mappa v�laszt�s�hoz kattintson a Tall�z�s gombra.$\r$\nKattintson a Tov�bb-ra a folytat�shoz."
FunctionEnd
  
Function dataDirShow
  !insertmacro MUI_HEADER_TEXT "Adatb�zis �tvonala" "Adja meg a k�zponti adatb�szis el�r�si �tvonal�t!"
  !insertmacro MUI_INNERDIALOG_TEXT 1006 "A CashFlow k�zponti adatb�zis a k�vetkez� helyen tal�lhat�.$\r$\n$\r$\nM�sik mappa v�laszt�s�hoz kattintson a Tall�z�s gombra.$\r$\nKattintson a Telep�t�s-re a folytat�shoz."
FunctionEnd

Function appDirLeave
  StrCpy $appDir $INSTDIR
  ${GetBaseName} $appDir $appDirBaseName
FunctionEnd
  
Function dataDirLeave
  StrCpy $dataDir $INSTDIR
FunctionEnd
