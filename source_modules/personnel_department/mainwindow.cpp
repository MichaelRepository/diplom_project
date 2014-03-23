#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectionname = "wowdb";

    QApplication::desktop()->availableGeometry(1);

    /// подсказки
    ui->Edit_button1->                  setToolTip("Обновить данные записи");
    ui->Add_button1->                   setToolTip("Добавить новую запись в таблицу");
    ui->Delete_button1->                setToolTip("Удалить выделенные записи из таблицы");
    ui->Refresh_button1->               setToolTip("Обновить (перезапросить) данные в таблице");
    ui->To_group_button->               setToolTip("Перевод выбранных студентов в группу");
    ui->Change_state_button->           setToolTip("Установить новое состояние для выбранных студентов");
    ui->Change_course_button->          setToolTip("Установить курс для выделенных групп");
    ui->Master_filter_button->          setToolTip("Вызвать диалог для построения фильтра");
    ui->Filter_selected_button->        setToolTip("Отобрать выделенные записи");
    ui->Filter_unselected_button->      setToolTip("Отобрать невыделенные записи");
    ui->Filter_attribute_button->       setToolTip("Установить фильтр по значению атрибута");
    ui->Filter_group_attribute_button-> setToolTip("Установить фильтр по группе атрибутов");
    ui->Filter_checked_button->         setToolTip("Вклюить/выключить фильтрацию записей");
    ui->Switch_table_spec_button->      setToolTip("Переключиться на таблицу [Специальности]");
    ui->Switch_table_group_button->     setToolTip("Переключиться на таблицу [Группы]");
    ui->Switch_table_stud_button->      setToolTip("Переключиться на таблицу [Студенты]");

    /// загрузка таблиц стилей QSS
    /*QString qcss = "";
    QFile filecss(":/style/WowStyle.qss");
    if ( filecss.open(QIODevice::ReadOnly) )
    {
        qcss = filecss.readAll();
        filecss.close();
        messdlg.     setStyleSheet(qcss);
        dbmessdlg.   setStyleSheet(qcss);
        authorizedlg.setStyleSheet(qcss);
        this->setStyleSheet(qcss);
    }*/

    /// создание меток, помещаемых в статус бар
    Status_label                = new QLabel(this);
    Status_label_curtable       = new QLabel(this);
    Status_label_count_rows     = new QLabel(this);
    Status_label_count_selected = new QLabel(this);
    Status_search_edit          = new QLineEdit(this);
    /// добавление меток в статус бар
    ui->statusBar->addPermanentWidget(Status_label);
    ui->statusBar->addPermanentWidget(Status_search_edit);
    ui->statusBar->addPermanentWidget(Status_label_curtable);
    ui->statusBar->addPermanentWidget(Status_label_count_rows);
    ui->statusBar->addPermanentWidget(Status_label_count_selected,1);

    Status_label->setMinimumWidth(5);
    Status_label->setMaximumWidth(5);
    Status_search_edit->setMinimumWidth(170);
    Status_search_edit->setStyleSheet( "background: url(:/png/search-icon.png) "
                                       "100% 100% no-repeat; "
                                       "background-position:left; "
                                       "padding-left:18px; "
                                       "height:20px;");

    header = new QSpreadsheetHeaderView(Qt::Horizontal, this);                  /// установка собственного заголовка для столбцов
    header->setHighlightSections(true);
    ui->tableView->setHorizontalHeader(header);

    /// создание контекстного меню таблицы
    Table_record_edit   = new QAction(QIcon(":/svg/edit-icon.svg"), "Изменить запись", this);
    Table_record_add    = new QAction(QIcon(":/svg/plus-icon.svg"), "Добавить запись", this);
    Table_record_remove = new QAction(QIcon(":/svg/minus-icon.svg"),"Удалить записи", this);
    ui->tableView->addAction(Table_record_edit);
    ui->tableView->addAction(Table_record_add);
    ui->tableView->addAction(Table_record_remove);
    ui->tableView->setContextMenuPolicy(Qt::ActionsContextMenu);                /// установка типа меню - отображать список действий

    /// создание загрузчика параметров приложения
    setting = new QSettings("../myapp.ini",QSettings::IniFormat,this);
}

MainWindow::~MainWindow()
{
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    db.close();

    query->clear();
    querymodel->deleteLater();

    delete ui;
}

void MainWindow::initTables()
{
    QStringList headerList;
/// подготовить таблицу - специальности
    specialitytable.setConnectionName(connectionname);
    specialitytable.setSelect ("* FROM speciality ");
    specialitytable.setWhere  ("");
    specialitytable.setOrderBy("idspeciality");
    headerList << "Шифр"
               << "Аббревиатура"
               << "Наименование"
               << "Период обучения"
               << "Базируется на"
               << "Специализация";
    specialitytable.setAlterFieldsName(headerList);
    headerList.clear();
/// подготовить таблицу - группы
    grouptable.setConnectionName(connectionname);
    grouptable.setSelect ("* FROM groups ");
    grouptable.setWhere  ("speciality = ? ");
    grouptable.setOrderBy("idgroup");
    headerList << "#"
               << "Специальность"
               << "Наименование"
               << "Год формирования"
               << "Форма обуч."
               << "Бюджет"
               << "Курс";
    grouptable.setAlterFieldsName(headerList);
    headerList.clear();
/// подготовить таблицу - студенты
    studenttable.setConnectionName(connectionname);
    studenttable.setSelect("student.subject, "
                           "student.numbertestbook, "
                           "subject.surname, "
                           "subject.name, "
                           "subject.patronymic, "
                           "subject.sex, "
                           "groups.name AS groupname, "
                           "groups.course, "
                           "groups.form, "
                           "groups.budget, "
                           "subject.datebirth, "
                           "subject.placebirth, "
                           "'...' AS citizenship, "
                           "'...' AS residence, "
                           "'...' AS passports, "
                           "'...' AS represent, "
                           "'...' AS education, "
                           "'...' AS privileges, "
                           "'...' AS moreinf "
                           "FROM student "
                           "LEFT JOIN groups  ON (student.group = groups.idgroup) "
                           "LEFT JOIN subject ON (student.subject = subject.idsubject) "
                           );
    studenttable.setWhere("student.group = ?");
    studenttable.setOrderBy("subject");
    headerList << "#"
               << "№ зачетки"
               << "Фамилия"
               << "Имя"
               << "Отчество"
               << "Пол"
               << "Группа"
               << "Курс"
               << "Форма обучения"
               << "Бюджет"
               << "Дата рождения"
               << "Место рождения"
               << "Гражданство"
               << "Проживание"
               << "Паспорт"
               << "Представители"
               << "Образование"
               << "Льготы"
               << "Дополнительно";
    studenttable.setAlterFieldsName(headerList);

/// подготовить таблицу - список гражданств
    citizenship.setConnectionName(connectionname);
    citizenship.setSelect("DISTINCT idcitizenship, citizenshipname "
                          "FROM citizenshiplist "
                          "INNER JOIN citizenshipforsubject "
                          "ON ( citizenshipforsubject.citizenship = citizenshiplist.idcitizenship)");
    citizenship.setWhere("citizenshipforsubject.subject = ?");
    citizenship.setOrderBy("idcitizenship");
    headerList << "#" << "Гражданство";
    citizenship.setAlterFieldsName(headerList);
    headerList.clear();

/// ВАЖНО - установка глобальной таблицы
    globaltable = &specialitytable;

}

void MainWindow::init_sys()
{
    /// создание загрузочного окна
    QPixmap pixmap(":SplashScreen.jpg");
    pixmap.scaledToHeight (100,  Qt::SmoothTransformation);
    pixmap.scaledToWidth  (100,  Qt::SmoothTransformation);
    splashwindow = new QSplashScreen(pixmap);
    splashwindow->setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint |
                                 Qt::WindowStaysOnTopHint);
    splashwindow->show();
    splashwindow->showMessage("Start");
    splashwindow->setEnabled(false);                                            /// делаю окно неактивным, чтобы при клике окно не закрывалось
    splashwindow->showMessage("Loaded modules");
    QString str;

    /// подключение к базе данных
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QMYSQL",connectionname);                    /// проверка наличия драйвера QMYSQL
    if (!db.isValid() )
    {
        dbmessdlg.showdbmess(db.lastError());
        exit(0);
    }

    /// авторизационные данные группы операторов АИС
    /// загрузка из файла
    setting->beginGroup("SqlDatabase");
    db.setHostName     (setting->value("HostName",      "").toString());
    db.setDatabaseName (setting->value("DatabaseName",  "").toString());
    db.setUserName     (setting->value("UserName",      "").toString());
    db.setPassword     (setting->value("Password",      "").toString());
    setting->endGroup();

    /// открытие соединеня
    if(!db.open())
    {
        str = "соединение с БД не установлено";
        dbmessdlg.showdbmess(db.lastError());
        exit(0);
    }
    str = "соединение c БД установлено";
    splashwindow->showMessage(str);
    QApplication::processEvents();

    /// авторизация пользователя
    authorizedlg.setconnectionname(connectionname);                             /// передать имя подключения
    while(authorizedlg.getstate() == REPEAT) {                                  /// проверять состояние авторизации
        authorizedlg.exec();                                                    /// вывод диалога авторизации
    }
    if(authorizedlg.getstate() == CANCEL) {exit(0);}                            /// во время авторизации произошла ошибка или было отменено действие
    userid = authorizedlg.getuserid();                                          /// получен идентификатор пользователя

    /// создание объектов для обработки и визуализации данных,
    /// получаемых SQL запросом
    query       = new QSqlQuery(db);                                            /// создание и привязка запроса к подключению
    querymodel  = new QSqlQueryModel();                                         /// создание модели данных
    ui->tableView->setModel(querymodel);                                        /// привязка вьювера и модели

    /// обработка выделения строк
    QObject::connect (ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                      this,                            &MainWindow::tableView_items_selected);


    initTables();                                                               /// ИНИЦИАЛИЗАЦИЯ ТАБЛИЦ
    set_current_table(SPECIALITY, BUTTONMODE);                                  /// установить текущую таблицу

    /// создание окна для отображения подтаблиц
    subtablewidget = new SubTableWidget(connectionname, this);                  /// установка флага - "контекстное меню", для диалога субтаблиц
    subtablewidget->setWindowFlags(Qt::Popup);

    splashwindow->finish(this);
    this->show();
}

QString MainWindow::get_connect_name()
{
    return connectionname;
}

void MainWindow::refresh_menu()
{
    /// переключить состояние кнопок панели меню, зависимых от активной таблицы
    switch (currenttable){
    case SPECIALITY:
        ui->Switch_table_spec_button->setChecked(true);
        ui->To_group_button->setEnabled(false);
        ui->Change_state_button->setEnabled(false);
        ui->Change_course_button->setEnabled(false);
        Status_label_curtable->setText("Таблица: Специальность ");
        break;
    case GROUP:
        ui->Switch_table_group_button->setChecked(true);
        ui->To_group_button->setEnabled(false);
        ui->Change_state_button->setEnabled(false);
        ui->Change_course_button->setEnabled(true);
        Status_label_curtable->setText("Таблица: Группа ");
        break;
    case STUDENT:
        ui->Switch_table_stud_button->setChecked(true);
        ui->To_group_button->setEnabled(true);
        ui->Change_state_button->setEnabled(true);
        ui->Change_course_button->setEnabled(false);
        Status_label_curtable->setText("Таблица: Студент ");
        break;
    }
    /// определить число выделенных записей активной таблицы
    int countselectedrows = ui->tableView->selectionModel()->selectedRows().size();
    bool state = (countselectedrows == 1);
    //УСТАНОВИТЬ ТЕКУЩУЮ СТРОКУ В ГЛОБАЛЬНОЙ ТАБЛИЦЕ
    if(state) globaltable->setCurrentRow(ui->tableView->selectionModel()->selectedRows().first().row());
    /// переключить состояние кнопок и элементов контекстного меню, зависимых от числа выделенных записей
    ui->Edit_button1->setEnabled(state);
    Table_record_edit->setEnabled(state);
    /// обновить надписи в статус баре
    Status_label_count_rows->setText    (" Число записей: "+ QString::number(query->size())+" ");
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));
}

void MainWindow::set_current_table(Tables table, ModeSwitchingTable mode)
{
    /// формирование запросов, относительно текущей таблицы
    switch(table){
    case SPECIALITY:
        specialitytable.execQuery(true,false,false,true);
        globaltable = &specialitytable;
        break;
    case GROUP:       
        if(mode == BUTTONMODE)
            grouptable.execQuery(true,false,false,true);
        else{
            QVariantList bindvalues;
            /* можно и так сделать
            grouptable.setWhere("WHERE speciality = ? ");
            */
            bindvalues << specialitytable.getCurrentRecordFieldValue("idspeciality");
            grouptable.bindValues(bindvalues);
            grouptable.execQuery(true,true,false,true);
        }
        globaltable = &grouptable;
        break;
    case STUDENT:
        if(mode == BUTTONMODE)
            studenttable.execQuery(true,false,false,true);
        else{
            QVariantList bindvalues;
            /* можно и так сделать
            grouptable.setWhere("WHERE student.group = ?");
            */
            bindvalues << grouptable.getCurrentRecordFieldValue("idgroup");
            studenttable.bindValues(bindvalues);
            studenttable.execQuery(true,true,false,true);
        }
        globaltable = &studenttable;
        break;
    }

    currenttable = table;

    globaltable->displayTable(ui->tableView);
    globaltable->setCurrentRow(-1);
    globaltable->displayTable(ui->tableView);

    /*if(table == STUDENT)
         header->hideSection(0);
    else header->showSection(0);*/

    refresh_menu();
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid() || currenttable == STUDENT)
        return ;
    Tables nexttable;

    //УСТАНОВИТЬ ВЫДЕЛЕННУЮ СТРОКУ КАК ТЕКУЩУЮ В ГЛОБАЛЬНОЙ ТАБЛИЦЕ
    globaltable->setCurrentRow(ui->tableView->selectionModel()->selectedRows().first().row());

    switch(currenttable){
    case SPECIALITY: nexttable = GROUP;
        break;
    case GROUP: nexttable = STUDENT;
        break;
    }

    set_current_table(nexttable, MOUSEMODE);                                    /// переключение таблиц
}

void MainWindow::on_Switch_table_spec_button_clicked()
{
    set_current_table(SPECIALITY, BUTTONMODE);
}

void MainWindow::on_Switch_table_group_button_clicked()
{
    set_current_table(GROUP, BUTTONMODE);
}

void MainWindow::on_Switch_table_stud_button_clicked()
{
    set_current_table(STUDENT, BUTTONMODE);
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    if(currenttable != STUDENT)   return ;                                      /// только для таблицы студенты
    if(index.column() < 12)       return ;                                      /// только для 12-19 столбцов таблицы

    int idsubject = query->value("subject").toInt();

    QString str1 = query->value("surname").toString();
    QString str2 = index.model()->headerData(index.column(),
                                             Qt::Horizontal).toString();
    subtablewidget->setTitleText(str1+" >> "+str2);
    SubTable subtable;
    switch(index.column()){
    case 12: subtable = CITIZENSHIP;
        break;
    case 13: subtable = RESIDENCE;
        break;
    case 14: subtable = PASSPORT;
        break;
    case 15: subtable = REPRESENT;
        break;
    case 16: subtable = EDUCATION;
        break;
    case 17: subtable = PRIVILEGES;
        break;
    /*case 18: subtable = PROGRESS;
        break;*/
    case 18: subtable = MOREINF;
        break;
    default: return ;
    }

    /// определяется точка вывода контекстного меню
    /// если позиция курсора по Y больше половины высоты десктопа + 200px
    /// выводить контекстное меню над курсором иначе под курсором
    QPoint pos = cursor().pos();
    if(cursor().pos().y() + subtablewidget->height() > QApplication::desktop()->availableGeometry(0).height())
        pos.setY(pos.y() - subtablewidget->height());
    ///тоже самое по X
    if(cursor().pos().x() + subtablewidget->width() > QApplication::desktop()->availableGeometry(0).width())
        pos.setX(pos.x() - subtablewidget->width());

    subtablewidget->move(pos);
    /// вывод диалогового окна с суб таблицей в качестве контекстного меню
    subtablewidget->ExecSqlQuery(subtable,idsubject);
    subtablewidget->show();

}

void MainWindow::tableView_items_selected()
{
    refresh_menu();
}

void MainWindow::add_new_record()                                               /// добавить запись
{

}

void MainWindow::remove_records()                                               /// удалить записи
{

}

void MainWindow::edit_records()                                                 /// изменить записи
{
    /// определить выбранную для редактирования запись
    QModelIndexList rowslist = ui->tableView->selectionModel()->selectedRows(0);/// получить список всех строк в которых выделен 0-й столбец
    if(rowslist.size() != 1) return ;                                           /// список содержит не одну строку -> завершить
    int row = rowslist[0].row();                                                /// получить номер выделенной строки
    query->seek(row);

    EditRecordModel recordmodel;                                                /// модель данных для записи таблицы
    QList<QString> reglist;                                                     /// список регулярных выражений для валиадторов
    QMap<QString, SubTableWidget *> tableattributelist;                         /// список спецатрибутов, являющихся субтаблицами. список(атрибут, визуализатор субтаблицы)

    switch(currenttable){
    case SPECIALITY:
        /// подготовка модели данных записи (атрибут, значение)
        recordmodel.modelAddRow("Шифр",            query->value("idspeciality"));
        recordmodel.modelAddRow("Аббревиатура",    query->value("abbreviation"));
        recordmodel.modelAddRow("Наименование",    query->value("name"));
        recordmodel.modelAddRow("Период обучения", query->value("periodeducation"));
        recordmodel.modelAddRow("Базируется на",   query->value("basedon"));
        recordmodel.modelAddRow("Специализация",   query->value("specialization"));
        /// подготовка списка регулярных выражений для валидатора
        reglist << "\\d{1,6}"
                << "[\\w\\s]{1,50}"
                << "[\\w\\s]{1,50}"
                << "\\d{1,3}"
                << "[\\w\\s]{1,100}"
                << "[\\w\\s]{1,100}";
        break;
    case GROUP:{
        /// подготовка модели данных записи (атрибут, значение)
        recordmodel.modelAddRow("Специальность",   query->value("speciality"));
        recordmodel.modelAddRow("Наименование",    query->value("name"));
        recordmodel.modelAddRow("Год создания",    query->value("yearformation"));
        recordmodel.modelAddRow("Форма обучения",  query->value("form"));
        recordmodel.modelAddRow("Бюджет",          query->value("budget"));
        recordmodel.modelAddRow("Курс",            query->value("course"));
        /// подготовка списка регулярных выражений для валидатора
        reglist << "\\d{0,6}"
                << "[\\w\\s]{0,50}"
                << "[\\w\\s]{0,50}"
                << "\\d{0,3}"
                << "[\\w\\s]{0,100}"
                << "[\\w\\s]{0,100}";


        /// создание вьювера для внешней таблицы
        SubTableWidget* subtableviewer = new SubTableWidget(connectionname);    /// создается виджет для отображения субтаблицы
        QStringList headerlist;                                                 /// список заголовков таблицы
        headerlist << "Шифр"
                   << "Аббревиатура";
        subtableviewer->setHeadersNameList(headerlist);                         /// установлен список заголовков
        subtableviewer->setDisplayedField("idspeciality");                      /// отображать в редакторе шифр специальности
        /// выполнение запроса для внешней таблицы
        subtableviewer->ExecSqlQuery("SELECT "
                                        "idspeciality, "
                                        "abbreviation "
                                     "FROM "
                                        "speciality "
                                     "ORDER BY "
                                        "idspeciality");
        /// позиционирование на текущую строку во внешней таблице по списку атр+знач
        QList<QPair<QString,QVariant> > attrlist;
        /// строка будет выбрана по значению атрибута speciality
        attrlist.append(qMakePair(QString("idspeciality"), query->value("speciality") ));
        if(!subtableviewer->setCurrentRow(attrlist)){
            messdlg.settitl("Ошибка");
            messdlg.settext("Не возможно обратиться к записи в таблице Группы");
            return ;
        }

        /// формирование списка атрибутов - ссылок на таблицы для редактора
        tableattributelist.insert("Специальность", subtableviewer);             /// специальность - ссылка на таблицу специальности

        break;
    }
    case STUDENT:{
        QSqlDatabase db = QSqlDatabase::database(connectionname);
        if(!db.open()){
            dbmessdlg.showdbmess(db.lastError());
            return ;
        }

        /// сохраняю идентификатор группы
        recordmodel.modelAddRow("Номер зачетки",    query->value("numbertestbook"));
        recordmodel.modelAddRow("Фамилия",          query->value("surname"));
        recordmodel.modelAddRow("Имя",              query->value("name"));
        recordmodel.modelAddRow("Отчество",         query->value("patronymic"));
        recordmodel.modelAddRow("Пол",              query->value("sex"));
        recordmodel.modelAddRow("Группа",           query->value("groupname"));
        recordmodel.modelAddRow("Дата рождения",    query->value("datebirth"));
        recordmodel.modelAddRow("Место рождения",   query->value("placebirth"));

        /// подготовка списка регулярных выражений для валидатора
        reglist << ""
                << ""
                << ""
                << ""
                << ""
                << "";

        /// создание вьювера для таблицы группы
        SubTableWidget *grouptableviewer = new SubTableWidget(connectionname);
        QStringList headerlist;
        headerlist  << "#"
                    << "Специальность"
                    << "Наименование"
                    << "Год формирования"
                    << "Форма обуч."
                    << "Бюджет"
                    << "Курс";
        grouptableviewer->setHeadersNameList(headerlist);
        grouptableviewer->setDisplayedField("name");
        grouptableviewer->ExecSqlQuery("SELECT "
                                           "groups.idgroup, "
                                           "speciality.name AS specname, "
                                           "groups.name, "
                                           "groups.yearformation, "
                                           "groups.form, "
                                           "groups.budget, "
                                           "groups.course "
                                       "FROM "
                                           "groups, "
                                           "speciality "
                                       "WHERE speciality.idspeciality = groups.speciality "
                                       "ORDER BY idgroup");
        /// позиционирование на текущую строку во внешней таблице по списку атр+знач
        QList<QPair<QString,QVariant> > attrlist;
        /// строка будет выбрана по значению атрибута idgroup
        /// получение номера группы в которой обучается студент
        QSqlQuery queryforgroup(db);
        queryforgroup.prepare("SELECT "
                                "student.group "
                              "FROM "
                                "student "
                              "WHERE subject = :id");
        queryforgroup.bindValue(":id",query->value("subject"));
        if(!queryforgroup.exec()){
            dbmessdlg.showdbmess(queryforgroup.lastError());
            return ;
        }
        queryforgroup.first();
        attrlist.append(qMakePair(QString("idgroup"), queryforgroup.value("group") ));
        if(!grouptableviewer->setCurrentRow(attrlist)){
            messdlg.settitl("Ошибка");
            messdlg.settext("Не возможно обратиться к записи в таблице Группы");
            return ;
        }

        grouptableviewer->findRecordByAttributeList(attrlist);
        tableattributelist.insert("Группа",grouptableviewer);
        break;
    }
    }
    dlgrecordedit.setModel(&recordmodel, &reglist, &tableattributelist);

    this->clearFocus();
    dlgrecordedit.exec();


    //удаление временных данных
    QMap<QString, SubTableWidget*>::const_iterator i = tableattributelist.constBegin();
    while (i != tableattributelist.constEnd()) {
        delete i.value();
        ++i;
    }

    /// ОБНОВИТЬ текущую таблицу!!!!


    tableattributelist.clear();
    query->first();
}

void MainWindow::on_Edit_button1_clicked()
{
    edit_records();
}
