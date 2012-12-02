#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T07:37:04
#
#-------------------------------------------------

QT       += core gui
QT       += network

CONFIG   += static

TARGET = srh
TEMPLATE = app
DESTDIR = ../srh/installer

RESOURCES = icons.qrc

SOURCES += main.cpp\
    mainwindow.cpp \
    tabledata.cpp \
    tableview.cpp \
    tablemodel.cpp \
    sumtableview.cpp \
    sumtablemodel.cpp \
    splashscreen.cpp \
    itemdelegate.cpp \
    messages.cpp \
    settings.cpp \
    fileaccess.cpp \
    sumtabledata.cpp \
    textedit.cpp \
    dateselect.cpp \
    math.cpp \
    combobox.cpp \
    search.cpp \
    setkey.cpp \
    pathes.cpp \
    mail.cpp

HEADERS  += mainwindow.h \
    tabledata.h \
    tableview.h \
    tablemodel.h \
    debug.h \
    sumtablemodel.h \
    sumtableview.h \
    splashscreen.h \
    itemdelegate.h \
    messages.h \
    settings.h \
    fileaccess.h \
    sumtabledata.h \
    textedit.h \
    dateselect.h \
    math.h \
    main.h \
    combobox.h \
    search.h \
    setkey.h \
    pathes.h \
    mail.h

FORMS    +=
