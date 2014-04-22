#include "mainwindow.h"

#include <QApplication>
#include <QtGui>


int main(int argc, char *argv[])
{
    /// загрузка библиотек
    QLibrary qsqlmysql("../../../files/box_library_5.2.1/sqldrivers/qsqlmysql.dll");
    QLibrary libmysql ("../../../files/box_library_5.2.1/libmysql.dll");
    //qDebug() << qsqlmysql.load();
    //qDebug() << libmysql.load();

    QApplication a(argc, argv);

    MainWindow win;
    win.init_sys();

    return a.exec();
}
