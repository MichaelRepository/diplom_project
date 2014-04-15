#ifndef AUTHORIZDLG_H
#define AUTHORIZDLG_H

#include <QDialog>

namespace Ui {
class AuthorizDlg;
}

class AuthorizDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AuthorizDlg(QWidget *parent = 0);
    ~AuthorizDlg();

    QString userName();
    QString userPass();

    void closeEvent(QCloseEvent *);
private slots:
    void on_cancelbt_clicked();
    void on_authorizbt_clicked();

    void on_checkBox_clicked();

private:
    Ui::AuthorizDlg *ui;
};

#endif // AUTHORIZDLG_H
