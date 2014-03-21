#-------------------------------------------------
#
# Project created by QtCreator 2013-12-08T10:40:41
#
#-------------------------------------------------

QT += core gui sql
QT += svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    messdlg.cpp \
    dbmessdlg.cpp \
    wowsqlmodel.cpp \
    authorizdlg.cpp \
    QSpreadsheetHeaderView.cpp \
    dialogeditrecord.cpp \
    editrecordmodel.cpp \
    recorddelegate.cpp \
    subtablewidget.cpp \
    myminiorm.cpp

HEADERS  += mainwindow.h \
    messdlg.h \
    dbmessdlg.h \
    wowsqlmodel.h \
    authorizdlg.h \
    QSpreadsheetHeaderView.h \
    dialogeditrecord.h \
    editrecordmodel.h \
    recorddelegate.h \
    subtablewidget.h \
    myminiorm.h

FORMS    += mainwindow.ui \
    messdlg.ui \
    dbmessdlg.ui \
    authorizdlg.ui \
    dialogeditrecord.ui \
    subtablewidget.ui

RESOURCES += ../../resfile/src.qrc

RC_FILE = ../../resfile/for_win_ico.rc
