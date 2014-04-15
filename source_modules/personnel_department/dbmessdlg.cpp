#include "dbmessdlg.h"
#include "ui_dbmessdlg.h"

#include <QTextCodec>

dbMessDlg::dbMessDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbMessDlg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | /*Qt::FramelessWindowHint |*/ Qt::WindowStaysOnTopHint);

    QFont font;
    font.setPixelSize(17);
    ui->TitleLine->setFont(font);

    QIcon ico(":svg/iconmonstr-speech-bubble-11-icon.svg");
    this->setWindowIcon(ico);
    this->setWindowTitle("Сообщение");

    ui->InfoBox->setVisible(false);
    this->resize(width(),150);
    this->setMaximumHeight(150);
}

dbMessDlg::~dbMessDlg()
{
    delete ui;
}

void dbMessDlg::showdbmess(QSqlError lasterror)
{
    QString errtypestr = "";
    QSqlError::ErrorType errtypecod = lasterror.type();

    switch(errtypecod)
    {
    case QSqlError::NoError :
        errtypestr = "Без ошибок";
        break;
    case QSqlError::ConnectionError:
        if(lasterror.number() == 1045)
            errtypestr = "Ошибка авторизации";
        else
            errtypestr = "Ошибка подключения";
        break;
    case QSqlError::StatementError:
        errtypestr = "Ошибка в SQL запросе";
        break;
    case QSqlError::TransactionError:
        errtypestr = "Ошибка транзакции БД";
        break;
    case QSqlError::UnknownError:
        errtypestr = "Неизвестная ошибка БД";
        break;
    }

    QString errtitl;
    QString errinfo;


    errtitl = "Ошибка " + QString::number(lasterror.number())+
              "\nТип: "+errtypestr;
    errinfo = "database: "+lasterror.databaseText()+"\n"+
              "driver: "+lasterror.driverText();

    ui->TitleLine->setText(errtitl);
    ui->InfoBox->setPlainText(errinfo);

    exec();
}

void dbMessDlg::setAdditionText(QString text)
{
    ui->additionlabel->setText(text);
}

void dbMessDlg::on_OkButton_clicked()
{
    done(0);
}

void dbMessDlg::on_toolButton_clicked()
{
    ui->InfoBox->setVisible(!ui->InfoBox->isVisible());
    if(!ui->InfoBox->isVisible())
    {
        this->resize(width(),150);
        this->setMaximumHeight(150);
    }
    else
    {
        this->setMaximumHeight(250);
        this->resize(width(),250);
    }
}
