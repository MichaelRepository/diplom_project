#include "subtablewidget.h"
#include "ui_subtablewidget.h"

SubTableWidget::SubTableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubTableWidget)
{
    ui->setupUi(this);

    statusbar = new QStatusBar(this);
    standartheader = ui->tableView->horizontalHeader();
    header    = new QSpreadsheetHeaderView(Qt::Horizontal,this);
    movable   = true;                                                           /// разрешить перемещение окна
    ui->tableView->setHorizontalHeader(header);

    QLayout* layoutWidget = ui->gridLayout_2;
    layoutWidget->setContentsMargins(0,0,0,0);
    layoutWidget->setSpacing(0);
    layoutWidget->addWidget(statusbar);
}

SubTableWidget::~SubTableWidget()
{
    delete ui;
}

void SubTableWidget::setTable(MyTable *_table)
{
    table = _table;
    table->displayTable(ui->tableView);
}

void SubTableWidget::setTitleText(QString text)
{
    ui->label->setText(text);
}

/*void SubTableWidget::setDisplayedField(QString field)
{
    displayedfield = field;
}*/

void SubTableWidget::setDisplayMode(bool titleVisible,
                                    bool editbuttonpanelVisible,
                                    bool headerMenuVisible,
                                    bool dlgMovable,
                                    Qt::FocusPolicy tablefocuspolicy)
{
    movable = dlgMovable;
    ui->label->setVisible(titleVisible);
    ui->toolsframe->setVisible(editbuttonpanelVisible);
    ui->tableView->setFocusPolicy(tablefocuspolicy);
    if(!headerMenuVisible){
        //QHeaderView *header = new QHeaderView(Qt::Horizontal,this);
        header->setFocusPolicy(Qt::NoFocus);
        ui->tableView->setHorizontalHeader(standartheader);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->resizeRowsToContents();
    }
    QString style = "QAbstractItemView:item::selected{color:#fff;background-color: #3399FF;}";

    if(tablefocuspolicy == Qt::NoFocus) ui->tableView->setStyleSheet(style);
}

void SubTableWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(!movable) return;
    mpos = event->pos();
}

void SubTableWidget::mouseMoveEvent(QMouseEvent *event)
{
        QWidget::mouseMoveEvent(event);
        if(!movable) return;
        if (mpos.x() >= 0 && event->buttons() && Qt::LeftButton)
        {
            QPoint diff = event->pos() - mpos;
            QPoint newpos = this->pos() + diff;

            this->move(newpos);
        }
}

void SubTableWidget::mouseReleaseEvent(QMouseEvent *event)
{
        QWidget::mouseMoveEvent(event);
        if(!movable) return;
        /// очистить старое значение позиции нажатия мыши
        mpos = QPoint(-1, -1);
        QWidget::mouseReleaseEvent(event);
}

void SubTableWidget::on_tableView_clicked(const QModelIndex &index)
{
    //УСТАНОВИТЬ В ТАБЛИЦЕ ТЕКЩУЮ СТРОКУ
    table->setCurrentRow(index.row());
    emit newrowselected();                                                      /// сообщить - новая строка выделена
}

/*void SubTableWidget::ExecSqlQuery(SubTable table, int keyvalue)
{
    model->query().clear();                                                     /// очистка данных в запросе
    headernamelist.clear();                                                     /// очистка списка заголовков
    QString querystr;
    switch(table){
    case CITIZENSHIP:
        ///  запрос списка гражданств относительно текущего субъекта
        querystr =    "SELECT "
                           "citizenshiplist.idcitizenship, "
                           "citizenshiplist.citizenshipname "
                       "FROM   "
                           "citizenshiplist, "
                           "citizenshipforsubject "
                       "WHERE citizenshipforsubject.citizenship = citizenshiplist.idcitizenship "
                       "AND citizenshipforsubject.subject = :id";
        /// подготовка списка заголовков
        headernamelist << "#"
                       << "Гражданство";
        break;
    case RESIDENCE:
        /// запрос мест регистрации и прожвания
        querystr =     "SELECT  "
                           "idresidence, "
                           "postcode, "
                           "regionrepublic, "
                           "area, "
                           "city, "
                           "locality, "
                           "street, "
                           "house, "
                           "corps, "
                           "apartment, "
                           "registered "
                       "FROM residence "
                       "WHERE subject = :id ";
        /// подготовка списка заголовков
        headernamelist << "#"
                       << "Почтовый индекс"
                       << "Регион/республика"
                       << "Район"
                       << "Город"
                       << "Населенный пункт"
                       << "Улица"
                       << "Дом"
                       << "Корпус"
                       << "Квартира"
                       << "Проживание";
        break;
    case PASSPORT:
        querystr =     "SELECT "
                           "number, "
                           "series, "
                           "codedepartament, "
                           "issued, "
                           "dateissue "
                       "FROM passport "
                       "WHERE subject = :id";
        /// подготовка списка заголовков
        headernamelist << "Номер"
                       << "Серия"
                       << "Код подразделения"
                       << "Кем выдан"
                       << "Дата выдачи";
        break;
    case REPRESENT:
        querystr =    "SELECT "
                           "idrepresent, "
                           "surname, "
                           "name, "
                           "patronymic, "
                           "whofalls, "
                           "placejob, "
                           "post, "
                           "officephone, "
                           "mobilephone "
                       "FROM represent "
                       "WHERE subject = :id";
        /// подготовка списка заголовков
        headernamelist << "#"
                       << "Фамилия"
                       << "Имя"
                       << "Отчество"
                       << "Кем приходится"
                       << "Место работы"
                       << "Должность"
                       << "Рабочий телефон"
                       << "Мобильный телефон"
                       << "Номер субъекта";
        break;
    case EDUCATION:
        querystr =    "SELECT "
                           "education.ideducation, "
                           "typeeducation.nametypeeducation, "
                           "education.documenteducation, "
                           "education.seriesdocument, "
                           "education.numberdocument, "
                           "education.dateissue, "
                           "school.nameschool, "
                           "school.adressschool, "
                           "school.typeschool "
                       "FROM   education "
                       "LEFT JOIN typeeducation ON (education.typeeducation = typeeducation.idtypeeducation) "
                       "LEFT JOIN school ON (education.school = school.idschool) "
                       "WHERE (education.subject = :id) ";
        /// подготовка списка заголовков
        headernamelist << "#"
                       << "Тип образования"
                       << "Документ об образовании"
                       << "Серия документа"
                       << "Номер документа"
                       << "Дата выдачи документа"
                       << "Уч. заведения"
                       << "Адрес уч. заведения"
                       << "Тип уч. заведения";
        break;
    case PRIVILEGES:
       querystr =      "SELECT "
                           "privileges.id, "
                           "privilegescategory.namecategory, "
                           "privileges.namedoc, "
                           "privileges.numberdoc, "
                           "privileges.dateissure, "
                           "privileges.moreinfo "
                       "FROM privileges "
                       "LEFT JOIN privilegescategory ON (privileges.idcategory = privilegescategory.idcategory) "
                       "WHERE (privileges.subject = :id)";

        /// подготовка списка заголовков
        headernamelist << "#"
                       << "Категория"
                       << "Документ"
                       << "Номер документа"
                       << "Дата выдачи"
                       << "Дополнительно";
        break;

    case MOREINF:
        querystr =     "SELECT "
                           "idinf, "
                           "dataname, "
                           "value "
                       "FROM moreinf "
                       "WHERE subject = :id";
        /// подготовка списка заголовков
        headernamelist << "#"
                       << "Информация"
                       << "Описание";
        break;
    }
    QSqlDatabase db = QSqlDatabase::database(curconnectname);
    QSqlQuery query(db);
    query.prepare(querystr);
    query.bindValue(":id", keyvalue);                                           /// подставновка реального значения
    query.exec();
    model->setQuery(query);
    if(model->lastError().isValid()){
            dbmess->showdbmess(db.lastError());
    }

    applyTableHeaders();                                                        /// применить заголовки таблицы
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    subtable = table;                                                           /// запомнить текущую таблицу
    currentrow = 0;

}
*/

