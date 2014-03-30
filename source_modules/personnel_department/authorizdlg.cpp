#include "authorizdlg.h"
#include "ui_authorizdlg.h"

AuthorizDlg::AuthorizDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorizDlg)
{
    ui->setupUi(this);

    usergroup      = 2;                                                         /// оператор Отдела Кадров
    userid         = -1;                                                        /// несуществующий номер
    authorizestate = REPEAT;                                                    /// повтор вывода диалога

    /// создаются диалоговые окна для вывода сообщений
    dbmessage = new dbMessDlg(this);                                            /// выводит сообщения от СУБД
    message   = new QMessageBox(this);                                          /// выводит сообщения об ошибках в работе с данными

    ui->iconwidget->setStyleSheet("image: url(:/svg/user.svg);");
    QIcon ico(":/svg/business-card.svg");
    this->setWindowIcon(ico);
    this->setWindowTitle("Авторизация пользователя");

    setWindowFlags(Qt::Dialog | /*Qt::WindowTitleHint |*/ Qt::WindowStaysOnTopHint);

    /// установка валидатора для полей ввода
    /// используется одинаковый валидатор для логина и пароля
    QRegExp regexp("\\w+");
    QValidator* validator = new QRegExpValidator(regexp,this);
    ui->lineEdit->  setValidator(validator);
    ui->lineEdit_2->setValidator(validator);
}

AuthorizDlg::~AuthorizDlg()
{
    delete ui;
}

AuthorizState AuthorizDlg::getstate()
{
     return authorizestate;
}

int AuthorizDlg::getuserid()
{
    return userid;
}

void AuthorizDlg::setconnectionname(QString connectionname)
{
    connectname = connectionname;
}

void AuthorizDlg::authorization()
{
    QSqlDatabase db = QSqlDatabase::database(connectname);
    if(!db.isValid()){
        dbmessage->showdbmess(db.lastError());
        authorizestate = CANCEL;
        return ;
    }
    /// валидация данных
    /// обеспечивается валидатором QRegExpValidator

    /// формирование запроса
    QSqlQuery* query = new QSqlQuery(db);
    query->prepare("SELECT iduser FROM operators WHERE login = :login "
                   "AND pass = :pass AND type = :type");
    query->bindValue(":login",ui->lineEdit->text());
    query->bindValue(":pass", ui->lineEdit_2->text());
    query->bindValue(":type", usergroup);
    if(!query->exec()){dbmessage->showdbmess(query->lastError());
                       authorizestate = CANCEL;  return ;}                      /// получение и проверка ответа

    if(!query->first()) {                                                       /// получить первую запись
        message->setWindowTitle("Сообщение");
        message->setText("Вход не выполнен. Введены некорректные авторизационные данные!\n"+
                         query->value("iduser:").toString());
        message->setButtonText(1,"ясно");
        message->exec();
        authorizestate = REPEAT;
        return ;
    }
    bool intisvalid;
    int idvalue = query->value("iduser").toInt(&intisvalid);
    if(!intisvalid){
        message->setWindowTitle("Сообщение");
        message->setText("Ошибка, программа будет закрыта!\n"
                         "Получен неверный идентификатор. id:"+
                         query->value("iduser:").toString());
        message->setButtonText(1,"ясно");
        message->exec();
        authorizestate = CANCEL;
        return ;
    }

    authorizestate = COMPLETED;
    userid = idvalue;
    return ;
}

void AuthorizDlg::on_authorizbt_clicked()
{
    authorization();
    done(1);
}

void AuthorizDlg::on_cancelbt_clicked()
{

    authorizestate = CANCEL;
    done(-1);
}

void AuthorizDlg::on_checkBox_toggled(bool checked)
{
    /// показать/скрыть символы при вводе пароля
    if (checked) ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
    else ui->lineEdit_2->setEchoMode(QLineEdit::Password);
}
