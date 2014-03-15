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

    ui->iconwidget->setStyleSheet("image: url(:/svg/iconmonstr-database-7-icon.svg);");
    QIcon ico(":svg/iconmonstr-speech-bubble-11-icon.svg");
    this->setWindowIcon(ico);
    this->setWindowTitle("Сообщение");
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
        errtypestr = "Ошибка подключения";
        break;
    case QSqlError::StatementError:
        errtypestr = "Ошибка в синтаксисе";
        break;
    case QSqlError::TransactionError:
        errtypestr = "Ошибка транзакции";
        break;
    case QSqlError::UnknownError:
        errtypestr = "Неизвестная ошибка";
        break;
    }

    QString errtitl;
    QString errinfo;


    errtitl = "Ошибка БД [" + QString::number(lasterror.number())+"]"+
              " Тип: "+errtypestr;
    errinfo = "database: "+lasterror.databaseText()+"\n"+
              "driver: "+lasterror.driverText();

    ui->TitleLine->setText(errtitl);
    ui->InfoBox->setPlainText(errinfo);

    exec();
}

void dbMessDlg::on_OkButton_clicked()
{
    done(0);
}
