#include "mainwindow.h"

#include <QApplication>
#include <QtGui>


int main(int argc, char *argv[])
{
    QLibrary qsqlmysql("../../../files/box_library_5.2.1/sqldrivers/qsqlmysql.dll");
    QLibrary libmysql ("../../../files/box_library_5.2.1/libmysql.dll");
    if (!qsqlmysql.load()) qDebug() << qsqlmysql.errorString();
    if (!libmysql.load())  qDebug() << libmysql.errorString();

    QApplication a(argc, argv);

    MainWindow win;
    win.init_sys();

    return a.exec();
}
