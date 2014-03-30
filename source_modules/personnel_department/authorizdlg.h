#ifndef AUTHORIZDLG_H
#define AUTHORIZDLG_H

#include <QDialog>
#include <QtSql>

#include <QRegExpValidator>
#include <QMessageBox>
#include "dbmessdlg.h"

namespace Ui {
class AuthorizDlg;
}

enum AuthorizState{COMPLETED, CANCEL, REPEAT};                                  /// состояние авторизации (завершено, отменено, повторить ввод)

class AuthorizDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AuthorizDlg(QWidget *parent = 0);
    ~AuthorizDlg();

    /**
    состояния:
        COMPLETED - авторизация прошла успешно, можно получить id
        CANCEL    - операция отменена id = -1
        REPEAT    - необходимо ввести авторизационные данные id = -1
    **/
    AuthorizState  getstate();                                                  /// получить состояние авторизации
    int  getuserid();                                                           /// получить id пользователя
    void setconnectionname(QString connectionname);                             /// установить имя подключения

private slots:
    void on_authorizbt_clicked();
    void on_cancelbt_clicked();
    void on_checkBox_toggled(bool checked);

private:
    void authorization();
    void closeEvent(QCloseEvent * event ){
        authorizestate = CANCEL;
    }

    QString connectname;                                                        /// имя подключения к БД
    int     usergroup;                                                          /// группа к которой относится пользователь данного приложения
    int     userid;                                                             /// идентификатор пользователя
    AuthorizState     authorizestate;                                           /// состояние авторизации
    Ui::AuthorizDlg *ui;

    dbMessDlg *dbmessage;
    QMessageBox* message;
};

#endif // AUTHORIZDLG_H
