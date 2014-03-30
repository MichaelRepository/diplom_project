#ifndef DBMESSDLG_H
#define DBMESSDLG_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class dbMessDlg;
}

/** диалоговое окно выводящее сообщения от СУБД **/
class dbMessDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit dbMessDlg(QWidget *parent = 0);
    ~dbMessDlg();

    void showdbmess(QSqlError lasterror);                                       // отобразить сообщение
    void setAdditionText(QString text);
private slots:
    void on_OkButton_clicked();

private:
        Ui::dbMessDlg *ui;
};

#endif // DBMESSDLG_H
