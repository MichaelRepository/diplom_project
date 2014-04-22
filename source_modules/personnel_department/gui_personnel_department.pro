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
    authorizdlg.cpp \
    QSpreadsheetHeaderView.cpp \
    dialogeditrecord.cpp \
    recorddelegate.cpp \
    subtablewidget.cpp \
    MyTable/myeditrecordmodel.cpp \
    MyTable/myinfoscheme.cpp \
    MyTable/mysqlrecord.cpp \
    MyTable/mytable.cpp \
    MyTable/mytablemodel.cpp \
    myfilterform.cpp \
    MyTable/myfiltermodel.cpp \
    MyTable/mydelegateforitemfilter.cpp

HEADERS  += mainwindow.h \
    messdlg.h \
    dbmessdlg.h \
    authorizdlg.h \
    QSpreadsheetHeaderView.h \
    dialogeditrecord.h \
    recorddelegate.h \
    subtablewidget.h \
    MyTable/myeditrecordmodel.h \
    MyTable/myinfoscheme.h \
    MyTable/mysqlfield.h \
    MyTable/mysqlrecord.h \
    MyTable/mytable.h \
    MyTable/mytablemodel.h \
    myfilterform.h \
    MyTable/myfilterdata.h \
    MyTable/myfiltermodel.h \
    MyTable/mydelegateforitemfilter.h

FORMS    += mainwindow.ui \
    messdlg.ui \
    dbmessdlg.ui \
    authorizdlg.ui \
    dialogeditrecord.ui \
    subtablewidget.ui \
    myfilterform.ui

RESOURCES += ../../resfile/src.qrc

RC_FILE = ../../resfile/for_win_ico.rc
