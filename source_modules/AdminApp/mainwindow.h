#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>

#include <QDesktopServices>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QtSql>
#include <QFile>

#include "QuaZIP/quazip.h"
#include "QuaZIP/quazipfile.h"
#include "MyReport/mydocumentodf.h"

#include "authorizdlg.h"
#include "dbmessdlg.h"
#include "reportconfigdialog.h"

namespace Ui {
class MainWindow;
}

class MyUserGroup
{
public:
    int id;
    QString name;
    QString info;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void init_sys();
    QMap<int, QString>  getGroupsNameList();

private slots:
    void on_tabWidget_currentChanged(int index);
    void on_addRepBt_clicked();
    void on_remRepBt_clicked();
    void on_editRepBt_clicked();
    void on_showRepBt_clicked();

    void repTableItemSelected(const QModelIndex &cur, const QModelIndex &prev);


    void on_saveRepBt_clicked();

private:
    Ui::MainWindow *ui;

    QSqlQueryModel *querymodel;

    void getOdtFileData(const QString &file,                                    /// прочитать данные из файла file
                        QByteArray &odtdata,                                    /// и сохранить их в odtdata
                        QByteArray &thumbdata);                                 /// получить из файла миниатюру
    void getUserGroups();                                                       /// получить данные по группам пользователей
    void showReportsTable();                                                    /// отобразить таблицу reports
    void addReport();                                                           /// добавить новый документ отчета
    void remReport();                                                           /// удалить документ отчета
    void editReport();                                                          /// заменить документ отчета
    void saveCopyRepTemp();
    void createReport();

    QString connectionname;                                                     /// имя подключения к бд
    QSettings* setting;                                                         /// для чтения параметров приложения
   // MyDocumentODF* mydocodf;                                                    /// объект для заполнения шаблона отчета
    QList<MyUserGroup> usergroups;                                              /// список групп пользователей
};

#endif // MAINWINDOW_H
