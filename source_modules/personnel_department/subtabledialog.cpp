#include "subtabledialog.h"
#include "ui_subtabledialog.h"

SubTableDialog::SubTableDialog(QString connectname, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);

    statusbar = new QStatusBar(this);

    QLayout* layoutWidget = ui->gridLayout;
    //layoutWidget ->addWidget(new QTextEdit);
    layoutWidget->setContentsMargins(1,1,1,1);
    layoutWidget->setSpacing(0);
    layoutWidget ->addWidget(statusbar);

    dbmess = new dbMessDlg(this);

    QSqlDatabase db = QSqlDatabase::database(connectname);
    if(!db.open() || !db.isValid()){                                            /// КРИТИЧНОЕ МЕСТО (решено)
        dbmess->showdbmess(db.lastError());
        exit(0);                                                                /// решено закрытием приложения
    }
    /// создание объектов для обработки и визуализации данных,
    /// получаемых SQL запросом
    header = new QSpreadsheetHeaderView(Qt::Horizontal,this);
    dbmess = new dbMessDlg(this);
    query  = new QSqlQuery(db);
    model  = new QSqlQueryModel(this);
    ui->tableView->setHorizontalHeader(header);
    ui->tableView->setModel(model);
}

SubTableDialog::~SubTableDialog()
{
    delete ui;
}

void SubTableDialog::setTitleText(QString text)
{
    ui->label->setText(text);
}

void SubTableDialog::ExecSqlQuery(SubTable table, int keyvalue)
{
    query->finish();                                                            /// очистка данных в запросе
    switch(table){
    case CITIZENSHIP:
        ///  запрос списка гражданств относительно текущего субъекта
        query->prepare(
                    "SELECT citizenshiplist.idcitizenship, citizenshiplist.citizenshipname "
                    "FROM   citizenshiplist, citizenshipforsubject "
                    "WHERE  citizenshipforsubject.citizenship = citizenshiplist.idcitizenship AND "
                    "citizenshipforsubject.subject = :id"
                    );
        break;
    case RESIDENCE:
        /// запрос мест регистрации и прожвания
        query->prepare(
                        "SELECT  idresidence, postcode, regionrepublic, area, city, "
                        "locality, street, house, corps, apartment, registered "
                        "FROM residence WHERE subject = :id "
                       );
        break;
    case PASSPORT:
        query->prepare(
                    "SELECT number, series, codedepartament, issued, dateissue "
                    "FROM passport WHERE subject = :id"
                    );
        break;
    case REPRESENT:
        query->prepare(
                    "SELECT idrepresent, surname, name, patronymic, whofalls, "
                    "placejob, post, officephone, mobilephone "
                    "FROM represent WHERE subject = :id"
                    );
        break;
    case EDUCATION:
        query->prepare("SELECT education.ideducation, typeeducation.nametypeeducation, "
                       "education.documenteducation, education.seriesdocument, education.numberdocument, "
                       "education.dateissue, school.nameschool, school.adressschool, "
                       "school.typeschool "
                       "FROM   education "
                       "LEFT JOIN typeeducation ON (education.typeeducation = typeeducation.idtypeeducation) "
                       "LEFT JOIN school		 ON (education.school = school.idschool) "
                       "WHERE (education.subject = :id) ");
        break;
    case PRIVILEGES:
        query->prepare("SELECT privileges.id, privilegescategory.namecategory, privileges.namedoc, "
                       "privileges.numberdoc, privileges.dateissure, privileges.moreinfo "
                       "FROM privileges "
                       "LEFT JOIN privilegescategory ON (privileges.idcategory = privilegescategory.idcategory) "
                       "WHERE (privileges.subject = :id)");
        break;
    /*case PROGRESS:
        query->prepare("SELECT * FROM progress WHERE subject = :id");
        break;*/
    case MOREINF:
        query->prepare("SELECT idinf, dataname, value "
                       "FROM moreinf WHERE subject = :id");
        break;
    }

    query->bindValue(":id", keyvalue);                                          /// подставновка реального значения
    if(!query->exec()){                                                         /// выполнение запроса
        dbmess->showdbmess(query->lastError());
        return ;
    }
    model->setQuery(*query);                                                    /// привязка модели запроса
    header->hideSection(0);

    switch(table){
    case CITIZENSHIP:
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Гражданство"));
        break;
    case RESIDENCE:
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("#"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Почтовый индекс"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Регион/республика"));
        ui->tableView->model()->setHeaderData(3,    Qt::Horizontal, tr("Район"));
        ui->tableView->model()->setHeaderData(4,    Qt::Horizontal, tr("Город"));
        ui->tableView->model()->setHeaderData(5,    Qt::Horizontal, tr("Населенный пункт"));
        ui->tableView->model()->setHeaderData(6,    Qt::Horizontal, tr("Улица"));
        ui->tableView->model()->setHeaderData(7,    Qt::Horizontal, tr("Дом"));
        ui->tableView->model()->setHeaderData(8,    Qt::Horizontal, tr("Корпус"));
        ui->tableView->model()->setHeaderData(9,    Qt::Horizontal, tr("Квартира"));
        ui->tableView->model()->setHeaderData(10,   Qt::Horizontal, tr("Проживание"));
        break;
    case PASSPORT:
        header->showSection(0);
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("Номер"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Серия"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Код подразделения"));
        ui->tableView->model()->setHeaderData(3,    Qt::Horizontal, tr("Кем выдан"));
        ui->tableView->model()->setHeaderData(4,    Qt::Horizontal, tr("Дата выдачи"));
        break;
    case REPRESENT:
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("#"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Фамилия"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Имя"));
        ui->tableView->model()->setHeaderData(3,    Qt::Horizontal, tr("Отчество"));
        ui->tableView->model()->setHeaderData(4,    Qt::Horizontal, tr("Кем приходится"));
        ui->tableView->model()->setHeaderData(5,    Qt::Horizontal, tr("Место работы"));
        ui->tableView->model()->setHeaderData(6,    Qt::Horizontal, tr("Должность"));
        ui->tableView->model()->setHeaderData(7,    Qt::Horizontal, tr("Рабочий телефон"));
        ui->tableView->model()->setHeaderData(8,    Qt::Horizontal, tr("Мобильный телефон"));
        ui->tableView->model()->setHeaderData(9,    Qt::Horizontal, tr("Номер субъекта"));
        break;
    case EDUCATION:
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("#"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Тип образования"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Документ об образовании"));
        ui->tableView->model()->setHeaderData(3,    Qt::Horizontal, tr("Серия документа"));
        ui->tableView->model()->setHeaderData(4,    Qt::Horizontal, tr("Номер документа"));
        ui->tableView->model()->setHeaderData(5,    Qt::Horizontal, tr("Дата выдачи документа"));
        ui->tableView->model()->setHeaderData(6,    Qt::Horizontal, tr("Уч. заведения"));
        ui->tableView->model()->setHeaderData(7,    Qt::Horizontal, tr("Адрес уч. заведения"));
        ui->tableView->model()->setHeaderData(8,    Qt::Horizontal, tr("Тип уч. заведения"));
        break;
    case PRIVILEGES:
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("#"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Категория"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Документ"));
        ui->tableView->model()->setHeaderData(3,    Qt::Horizontal, tr("Номер документа"));
        ui->tableView->model()->setHeaderData(4,    Qt::Horizontal, tr("Дата выдачи"));
        ui->tableView->model()->setHeaderData(5,    Qt::Horizontal, tr("Дополнительно"));
        break;
    /*case PROGRESS:
        break;*/
    case MOREINF:
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("#"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Информация"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Описание"));
        break;
    }

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    subtable = table;                                                           /// запомнить текущую таблицу

    this->exec();
}

void SubTableDialog::ExecSqlQuery(QString sqlstring, QString namekey)
{

}

void SubTableDialog::mousePressEvent(QMouseEvent *event)
{
    mpos = event->pos();
    QDialog::mousePressEvent(event);
}
void SubTableDialog::mouseMoveEvent(QMouseEvent *event)
{
        if (mpos.x() >= 0 && event->buttons() && Qt::LeftButton)
        {
            QPoint diff = event->pos() - mpos;
            QPoint newpos = this->pos() + diff;

            this->move(newpos);
        }
        QDialog::mouseMoveEvent(event);
}
void SubTableDialog::mouseReleaseEvent(QMouseEvent *event)
{
        /// очистить старое значение позиции нажатия мыши
        mpos = QPoint(-1, -1);
        QDialog::mouseReleaseEvent(event);
}
