#include "subtablewidget.h"
#include "ui_subtablewidget.h"

SubTableWidget::SubTableWidget(QString connectname, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubTableWidget)
{
    ui->setupUi(this);
    curconnectname = connectname;

    statusbar = new QStatusBar(this);

    QLayout* layoutWidget = ui->gridLayout_2;
    layoutWidget->setContentsMargins(0,0,0,0);
    layoutWidget->setSpacing(0);
    layoutWidget->addWidget(statusbar);

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
    model  = new QSqlQueryModel(this);
    ui->tableView->setHorizontalHeader(header);
    ui->tableView->setModel(model);

    movable = true;                                                             /// разрешить перемещение окна
    currentrow = 0;                                                             /// текущая строка
}

SubTableWidget::~SubTableWidget()
{
    delete ui;
}
void SubTableWidget::setTitleText(QString text)
{
    ui->label->setText(text);
}

void SubTableWidget::ExecSqlQuery(SubTable table, int keyvalue)
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
    /*case PROGRESS:
        query->prepare("SELECT * FROM progress WHERE subject = :id");
        break;*/
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

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    subtable = table;                                                           /// запомнить текущую таблицу
    currentrow = 0;

}

void SubTableWidget::ExecSqlQuery(QString sqlstring)
{
    model->query().finish();
    QSqlDatabase db = QSqlDatabase::database(curconnectname);
    QSqlQuery query(db);
    query.exec(sqlstring);
    model->setQuery(query);
    if(model->lastError().isValid()){
        dbmess->showdbmess(db.lastError());
    }

    applyTableHeaders();                                                        /// применить заголовки таблицы

    currentrow = 0;
}

void SubTableWidget::setHeadersNameList(QStringList &namelist)
{
    headernamelist = namelist;
}

void SubTableWidget::setDefaultAttributesList(QStringList &list)
{
    defaultattributes = list;
}

bool SubTableWidget::setCurrentRow(int row)
{
    if( model->query().isActive() && row < model->record().count() && row >= 0 ){
        currentrow = row;
        applyCurrentRow();
        return true;
    }
    else return false;
}

bool SubTableWidget::setCurrentRow(QList<QPair<QString, QVariant> > &searchoption)
{
    int row = findRecordByAttributeList(searchoption);

    if(row != -1){
        currentrow = row;
        applyCurrentRow();
        return true;
    }
    else return false;
}

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
        QHeaderView *header = new QHeaderView(Qt::Horizontal,this);
        header->setFocusPolicy(Qt::NoFocus);
        ui->tableView->setHorizontalHeader(header);
    }
    QString style = "QAbstractItemView:item::selected{color:#fff;background-color: #3399FF;}";

    if(tablefocuspolicy == Qt::NoFocus) ui->tableView->setStyleSheet(style);
}

QVariant SubTableWidget::getCurRecordAttributeValue(QString attribute)
{
    if(!model->query().isValid()) return QVariant();
    return model->record(currentrow).value(attribute);
}

QVariantList SubTableWidget::getDefaultAttributesValue()
{
    QVariantList resultlist;
    if (model->query().isValid() )
    for(int i = 0; i < defaultattributes.size(); i++)
        resultlist << model->record(currentrow).value(defaultattributes.at(i));

    return resultlist;
}

int SubTableWidget::findRecordByAttributeList(QList<QPair<QString, QVariant> > &attrlist)
{
    if(!model->query().isActive() || attrlist.size() == 0) return -1;

    int  currow = 0;
    bool curf = false;
    int  cura = 0;

    while(curf == false && currow < model->record().count())
    {
        while(curf == false && cura < attrlist.size())
        {
            QPair<QString, QVariant>* curoption = &attrlist[cura];
            curf = (model->record(currow).contains(curoption->first) &
                     model->record(currow).value(curoption->first) == curoption->second);
            cura ++;
        }
        cura = 0;
        currow++;
    }
    currow--;
    if(currow < model->record().count()) return currow;
    else return -1;
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
    if(!index.isValid() || currentrow == index.row()) return ;
    currentrow = index.row();
    emit newrowselected();                                                      /// сообщить - новая строка выделена
}

void SubTableWidget::applyTableHeaders()
{
    /// установка заголовков таблицы
    for(int i = 0; i < headernamelist.size(); ++i)
        ui->tableView->model()->setHeaderData(i, Qt::Horizontal, headernamelist.at(i) );
}

void SubTableWidget::applyCurrentRow()
{
    ui->tableView->selectionModel()->select(ui->tableView->model()->index(currentrow,0),
                                            QItemSelectionModel::ClearAndSelect |
                                            QItemSelectionModel::Rows);

    if(ui->tableView->focusPolicy() != Qt::NoFocus) ui->tableView->setFocus();
    emit newrowselected();                                                      /// сообщить - новая строка выделена
}


