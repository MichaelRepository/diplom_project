#include "authorizdlg.h"
#include "ui_authorizdlg.h"

AuthorizDlg::AuthorizDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorizDlg)
{
    ui->setupUi(this);

    /// установка валидатора для полей ввода
    /// используется одинаковый валидатор для логина и пароля
    QRegExp regexp("\\w+");
    QValidator* validator = new QRegExpValidator(regexp,this);
    ui->LoginEdit->setValidator(validator);
    ui->PassEdit ->setValidator(validator);
}

AuthorizDlg::~AuthorizDlg()
{
    delete ui;
}

QString AuthorizDlg::userName()
{
    return ui->LoginEdit->text();
}

QString AuthorizDlg::userPass()
{
    return ui->PassEdit->text();
}

void AuthorizDlg::closeEvent(QCloseEvent *)
{
    done(0);
}

void AuthorizDlg::on_cancelbt_clicked()
{
    done(0);
}

void AuthorizDlg::on_authorizbt_clicked()
{
    done(1);
}

void AuthorizDlg::on_checkBox_clicked()
{
    if(ui->checkBox->isChecked())
        ui->PassEdit->setEchoMode(QLineEdit::Normal);
    else
        ui->PassEdit->setEchoMode(QLineEdit::Password);
}
