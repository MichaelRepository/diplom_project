#include "messdlg.h"
#include "ui_messdlg.h"

MessDlg::MessDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessDlg)
{
    this->setWindowFlags(Qt::Popup | Qt::Window);
    //this->setWindowFlags(Qt::FramelessWindowHint );
    ui->setupUi(this);
}

MessDlg::~MessDlg()
{
    delete ui;
}

void MessDlg::setdata(QString titl, QString text, QString info)
{
    ui->TitleLine->setText(titl);
    ui->TextLine->setText(text);
    ui->InfoBox->setPlainText(info);
}

void MessDlg::settitl(QString titl)
{
    ui->TitleLine->setText(titl);
}

void MessDlg::settext(QString text)
{
    ui->TextLine->setText(text);
}

void MessDlg::setinfo(QString info)
{
    ui->InfoBox->setPlainText(info);
}

void MessDlg::on_OkButton_clicked()
{
    this->done(0);
}
