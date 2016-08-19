#-------------------------------------------------
#
# Project created by QtCreator 2016-02-24T16:23:37
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CsCode
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    commandedit.cpp \
    codeedit.cpp \
    codehlighter.cpp \
    codeparse.cpp \
    msgedit.cpp \
    commandhlighter.cpp \
    msghlighter.cpp \
    settingsdialog.cpp \
    imagetobindialog.cpp \
    updateconfig.cpp \
    crc.cpp

HEADERS  += mainwindow.h \
    commandedit.h \
    codeedit.h \
    codehlighter.h \
    codeparse.h \
    msgedit.h \
    commandhlighter.h \
    msghlighter.h \
    settingsdialog.h \
    imagetobindialog.h \
    updateconfig.h \
    crc.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    imagetobindialog.ui \
    updateconfig.ui

RESOURCES += \
    qrc.qrc
RC_FILE +=CsCode.rc