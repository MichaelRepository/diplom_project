#-------------------------------------------------
#
# Project created by QtCreator 2014-04-09T22:20:32
#
#-------------------------------------------------

QT       += core gui sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AdminApp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    authorizdlg.cpp \
    dbmessdlg.cpp \
    reportconfigdialog.cpp \
    MyReport/mydocumentodf.cpp

HEADERS  += mainwindow.h \
    authorizdlg.h \
    dbmessdlg.h \
    reportconfigdialog.h \
    MyReport/mydocumentodf.h

FORMS    += mainwindow.ui \
    authorizdlg.ui \
    dbmessdlg.ui \
    reportconfigdialog.ui

LIBS += -L"../../quaziplib/" -lquazip
INCLUDEPATH +=  "../../quaziplib"

RESOURCES += ../../resfile/src.qrc

RC_FILE = ../../resfile/for_win_ico.rc
