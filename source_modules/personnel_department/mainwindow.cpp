#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    userid = -1;

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

    /// создание окна для отображения подтаблиц
    subtablewidget = new SubTableWidget(this);                                  /// установка флага - "контекстное меню", для диалога субтаблиц
    subtablewidget->setWindowFlags(Qt::Popup);
    subtablewidget->setTitleText("");

    /// создание меток, помещаемых в статус бар
    Status_label_curtable       = new QLabel(this);
    Status_label_count_rows     = new QLabel(this);
    Status_label_count_selected = new QLabel(this);
    Status_search_edit          = new QLineEdit(this);

    connect(Status_search_edit, &QLineEdit::textEdited,
            this,               &MainWindow::searchStart);

    /// добавление меток в статус бар
    ui->statusBar->addPermanentWidget(Status_search_edit);
    ui->statusBar->addPermanentWidget(Status_label_curtable);
    ui->statusBar->addPermanentWidget(Status_label_count_rows);
    ui->statusBar->addPermanentWidget(Status_label_count_selected,1);
    ui->statusBar->setStyleSheet("QStatusBar::item{border:none;}");

    Status_search_edit->setMinimumWidth(170);
    Status_search_edit->setStyleSheet( "margin-left:5px;background: url(:/png/search-icon.png) "
                                       "100% 100% no-repeat; "
                                       "background-position:left; "
                                       "padding-left:18px; "
                                       "height:20px;");

    header = new QSpreadsheetHeaderView(Qt::Horizontal, this);                  /// установка собственного заголовка для столбцов
    header->setHighlightSections(true);

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

    delete ui;
}

void MainWindow::initTables()
{
/// создание источника метаданных
    metadatasource = new MyInfoScheme(connectionname, this);
    if(metadatasource->isError() )
    {
        dbmessdlg.setAdditionText("Приложение будет закрыто");
        dbmessdlg.showdbmess(metadatasource->lastQueryError());
        exit(0);
    }

/// создание таблиц
    residencetable   = new MyTable(metadatasource, connectionname);
    specialitytable  = new MyTable(metadatasource, connectionname);
    grouptable       = new MyTable(metadatasource, connectionname);
    studenttable     = new MyTable(metadatasource, connectionname);
    citizenshiptable = new MyTable(metadatasource, connectionname);

/// подготовить таблицу - проживание
    residencetable->appendField("idresidence",    "residence", false, false);
    residencetable->appendField("postcode",       "residence", true, true);
    residencetable->appendField("regionrepublic", "residence", true, true);
    residencetable->appendField("area",           "residence", true, true);
    residencetable->appendField("city",           "residence", true, true);
    residencetable->appendField("locality",       "residence", true, true);
    residencetable->appendField("street",         "residence", true, true);
    residencetable->appendField("house",          "residence", true, true);
    residencetable->appendField("corps",          "residence", true, true);
    residencetable->appendField("apartment",      "residence", true, true);
    residencetable->appendField("registered",     "residence", true, true);
    residencetable->appendField("subject",        "residence", false, false);

/// подготовить таблицу - специальности
    specialitytable->appendField("idspeciality",     "speciality", true, true);
    specialitytable->appendField("abbreviation",     "speciality", true, true);
    specialitytable->appendField("specialityname",   "speciality", true, true);
    specialitytable->appendField("periodeducation",  "speciality", true, true);
    specialitytable->appendField("basedon",          "speciality", true, true);
    specialitytable->appendField("specialization",   "speciality", true, true);

/// подготовить таблицу - группы
    grouptable->appendField("idgroup",      "groups", false, true);
    grouptable->appendField("speciality",   "groups", true,  true);
    grouptable->appendField("groupname",    "groups", true,  true);
    grouptable->appendField("yearformation","groups", true,  true);
    grouptable->appendField("form",         "groups", true,  true);
    grouptable->appendField("budget",       "groups", true,  true);

/// подготовить таблицу - студенты
    studenttable->appendField("subject",          "student", false, true);
    studenttable->appendField("numbertestbook",   "student", true,  true);
    studenttable->appendField("surname",          "subject", true,  true);
    studenttable->appendField("name",             "subject", true,  true);
    studenttable->appendField("patronymic",       "subject", true,  true);
    studenttable->appendField("sex",              "subject", true,  true);
    studenttable->appendField("groupname",        "groups",  false, true);
    studenttable->appendField("course",           "groups",  false, true);
    studenttable->appendField("budget",           "groups",  false, true);
    studenttable->appendField("datebirth",        "subject", true,  true);
    studenttable->appendField("placebirth",       "subject", true,  true);
    studenttable->appendField("studentgroup",     "student", true,  false);
    studenttable->appendField("idsubject",        "subject", false, false);
    studenttable->appendLink ("Проживание", "idsubject", "subject", residencetable);

/// подготовить таблицу - список гражданств
    citizenshiptable->appendField("idcitizenship",   "citizenshiplist", false, true);
    citizenshiptable->appendField("citizenshipname", "citizenshiplist", false, true);

/// инициализация таблиц
    bool    allinit    = true;

    allinit &= residencetable  ->initializeTable();
    allinit &= specialitytable ->initializeTable();
    allinit &= grouptable      ->initializeTable();
    allinit &= citizenshiptable->initializeTable();
    allinit &= studenttable    ->initializeTable();

    if(!allinit)
    {
        QMessageBox messbox;
        messbox.setWindowTitle("Ошибка инициализации данных");
        messbox.setText("Произошла ошибка в процессе инициализации таблиц\n"
                        "Приложение будет закрыто");
        messbox.setIcon(QMessageBox::Critical);
        messbox.addButton(QMessageBox::Ok);
        messbox.setButtonText(QMessageBox::Ok, "Ясно");
        messbox.exec();
        exit(0);
    }

/// ВАЖНО - установка глобальной таблицы
    dlgrecordedit   = new DialogEditRecord(connectionname, this);
    tablemodel      = new MyTableModel(this);
    subtablemodel   = new MyTableModel(this);
    globaltable     = specialitytable;
}

void MainWindow::init_sys()
{
    /// создание загрузочного окна
    QPixmap pixmap(":SplashScreen.jpg");
    pixmap.scaledToHeight (100,  Qt::SmoothTransformation);
    pixmap.scaledToWidth  (100,  Qt::SmoothTransformation);
    splashwindow = new QSplashScreen(pixmap);
    splashwindow->setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint);
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
    db.setPort         (setting->value("Port",          "").toInt());
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

    while(authorizedlg.exec() != 0 && userid == -1)
    {
        QSqlQuery query0(db);
        query0.prepare("SELECT iduser FROM operators WHERE login = :login "
                       "AND pass = :pass AND type = :type");
        query0.bindValue(":login", authorizedlg.userName());
        query0.bindValue(":pass",  authorizedlg.userPass());
        query0.bindValue(":type",  3);
        if(!query0.exec())
        {
            dbmessdlg.showdbmess(query0.lastError());
            break;
        }
        if(!query0.first())
        {
            QMessageBox messbox;
            messbox.setIcon(QMessageBox::Warning);
            messbox.setWindowTitle("Сообщение");
            messbox.setText("Указаны некорректные авторизационные данные!");
            messbox.addButton(QMessageBox::Ok);
            messbox.setButtonText(QMessageBox::Ok,"ясно");
            messbox.exec();
        }
        else if(query0.value(0).isValid())
        {
            userid = query0.value(0).toInt();
            break;
        }
    }
    if(userid == -1) exit(0);

    initTables();                                                               /// ИНИЦИАЛИЗАЦИЯ ТАБЛИЦ
    set_current_table(SPECIALITY, BUTTONMODE);                                  /// установить текущую таблицу

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
    ///УСТАНОВИТЬ ТЕКУЩУЮ СТРОКУ В ГЛОБАЛЬНОЙ ТАБЛИЦЕ
    if(state) globaltable->setCurrentRow(ui->tableView->selectionModel()->selectedRows().first().row());
    /// переключить состояние кнопок и элементов контекстного меню, зависимых от числа выделенных записей
    ui->Edit_button1->setEnabled(state);
    Table_record_edit->setEnabled(state);
    /// обновить надписи в статус баре
    Status_label_count_rows->setText    (" Число записей: "+ QString::number(globaltable->getRecordsCount())+" ");
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));

    ui->Filter_checked_button->setChecked(globaltable->isFiltered());
}

void MainWindow::set_current_table(Tables table, ModeSwitchingTable mode)
{
    /// формирование запросов, относительно текущей таблицы
    switch(table){
    case SPECIALITY:
        if(!specialitytable->updateData())
        {
            if(specialitytable->lastSqlError().isValid()){
                dbmessdlg.showdbmess(specialitytable->lastSqlError());
                return ;
            }
        }
        globaltable = specialitytable;
        break;
    case GROUP:
        if(!grouptable->updateData())
        {
            if(grouptable->lastSqlError().isValid()){
                dbmessdlg.showdbmess(grouptable->lastSqlError());
                return ;
            }
        }
        globaltable = grouptable;
        break;
    case STUDENT:
       if(!studenttable->updateData())
       {
           if(studenttable->lastSqlError().isValid()){
               dbmessdlg.showdbmess(studenttable->lastSqlError());
               return ;
           }
       }
        globaltable = studenttable;
        break;
    }

    currenttable = table;
    tablemodel->setTableObject(globaltable);
    ui->tableView->setModel(tablemodel);
    ui->tableView->setHorizontalHeader(header);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
    globaltable->setCurrentRow(-1);
    header->show();
    /// обработка выделения строк
    QObject::connect (ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                      this,                            &MainWindow::tableView_items_selected,
                      Qt::UniqueConnection);

    refresh_menu();
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid() || currenttable == STUDENT)
        return ;
    Tables nexttable;

    ///УСТАНОВИТЬ ВЫДЕЛЕННУЮ СТРОКУ КАК ТЕКУЩУЮ В ГЛОБАЛЬНОЙ ТАБЛИЦЕ
    globaltable->setCurrentRow(ui->tableView->selectionModel()->selectedRows().first().row());

    switch(currenttable){
    case SPECIALITY:{
        nexttable = GROUP;
        int currecord = specialitytable->getCurrentRowIndex();
        QString filtervalue = specialitytable->displayFieldValue(currecord,"idspeciality").toString();
        grouptable->setFilterForField("speciality", filtervalue);
        grouptable->setFilterActivity(true);
        break;
    }
    case GROUP:{
        nexttable = STUDENT;
        int currecord = grouptable->getCurrentRowIndex();
        QString filtervalue = grouptable->displayFieldValue(currecord,"idgroup").toString();
        studenttable->setFilterForTable("student.studentgroup = "+filtervalue);
        studenttable->setFilterActivity(true);
        /*
        QString filtervalue = grouptable->getCurrentRecordFieldValue("groupname").toString();
        studenttable->setFilterForField("groupname", filtervalue);
        */
        break;
    }
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
    if(!globaltable->displayedFieldIsLink(index.column()) ) return;
    MyLink * link = globaltable->getdisplayedLink(index.column() );
    if(link->table_1 == 0) return;
    QVariant key0_1val = globaltable->getFieldValue(link->key0_1);
    if(!key0_1val.isValid()) return;
    link->table_1->setFilterForField(link->key1_0, key0_1val.toString());
    link->table_1->setFilterActivity(true);
    /**
        танец с бубном:
        >задача: необходимо получить координаты точки в которую
         переместить виджет subtable.
         Точка должна быть расчитана относительно позиции ячейки.
        >проблема: не корректно определяется чилдрен виджет вьювера по
         заданным координатам.
        >возможные причины: заголовок таблици и первая строка являются одним
         элментом, тоесть секция хедера ячейка соотвествующего столбца - есть
         одна ячейка.
        >решение: получение координат visualRect иперемещение их в середину виджета,
         путём увеличения x на величину w, а y на h
    **/

    QRect cell = ui->tableView->visualRect(index);
    QPoint pos(cell.x(),cell.y());
    QPoint pos1(pos.x()+cell.width()/2,pos.y()+cell.height()/2);
    pos = ui->tableView->childAt(pos1)->mapToGlobal(pos);
    if(index.row() == 0)
        pos.setY(pos.y()+2*cell.height());
    else
        pos.setY(pos.y()+cell.height());

    pos.setX(pos.x()+20);

    /// определяется точка вывода контекстного меню
    /// если позиция курсора по Y больше половины высоты десктопа + 200px
    /// выводить контекстное меню над курсором иначе под курсором
    if(pos.y() + subtablewidget->height() > QApplication::desktop()->availableGeometry(0).height())
        pos.setY(pos.y() - subtablewidget->height());
    ///тоже самое по X
    if(pos.x() + subtablewidget->width() > QApplication::desktop()->availableGeometry(0).width())
        pos.setX(pos.x() - subtablewidget->width()-20);

    subtablemodel->setTableObject(link->table_1);
    link->table_1->updateData();
    subtablewidget->setDisplayMode(true,true,true,true);
    subtablewidget->setTableModel(subtablemodel);
    subtablewidget->move(pos);
    subtablewidget->show();
}

void MainWindow::tableView_items_selected()
{
    refresh_menu();
}

void MainWindow::add_new_record()                                               /// добавить запись
{
    dlgrecordedit->setRecord(globaltable->getEmptyRecordForInsert() );
    int res = dlgrecordedit->exec();
    if(res == 1)
    {
        if(!globaltable->setInsertDataFields())
        {
            dbmessdlg.showdbmess(globaltable->lastSqlError() );
        }
        globaltable->updateData();
        tablemodel->refresh();
    }
}

void MainWindow::remove_records()                                               /// удалить записи
{
    if(!globaltable->removeRecord(globaltable->getCurrentRowIndex() ))
    {
        // !!!
    }
    globaltable->updateData();
    tablemodel->refresh();
}

void MainWindow::edit_records()                                                 /// изменить записи
{
    dlgrecordedit->setRecord(globaltable->getRecord(globaltable->getCurrentRowIndex() ) );
    int res = dlgrecordedit->exec();
    if(res == 1)
    {
        if(!globaltable->setUpdateDataFields())
        {
            dbmessdlg.showdbmess(globaltable->lastSqlError());
        }
        globaltable->updateData();
        tablemodel->refresh();
    }
/**
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
**/
}

void MainWindow::refresh_table()
{
    if(!globaltable->updateData())
    {
        dbmessdlg.showdbmess(globaltable->lastSqlError());
    }
}

void MainWindow::on_Edit_button1_clicked()
{
    edit_records();
}

void MainWindow::on_Refresh_button1_clicked()
{
    refresh_table();
}

void MainWindow::on_Add_button1_clicked()
{
    add_new_record();
}

void MainWindow::on_Delete_button1_clicked()
{
    remove_records();
}

void MainWindow::on_Filter_checked_button_clicked()
{
    bool oldstate = globaltable->isFiltered();
    globaltable->setFilterActivity(!oldstate);
    bool newstate = globaltable->isFiltered();
    ui->Filter_checked_button->setChecked(newstate);
    if(newstate != oldstate)
    {
        globaltable->updateData();
        tablemodel->refresh();
    }
}

void MainWindow::searchStart()
{
    QString findvalue = Status_search_edit->text();
    QItemSelectionModel* selectionmodel = ui->tableView->selectionModel();
    QAbstractItemModel*  model          = ui->tableView->model();
    if(findvalue.size() < 2) {selectionmodel->clearSelection(); return;}
    QString fieldname;
    switch(currenttable){
    case SPECIALITY:
        fieldname = "abbreviation";
        break;
    case GROUP:
        fieldname = "groupname";
        break;
    case STUDENT:
       fieldname = "surname";
       break;
    }
    QList<int> result = globaltable->searchRecordByField(fieldname, findvalue);
    QList<int>::const_iterator itr;
    selectionmodel->clearSelection();
    for(itr = result.begin(); itr != result.end(); ++itr)
    {
        selectionmodel->select(model->index((*itr),0) ,
                               QItemSelectionModel::Rows |
                               QItemSelectionModel::Select);
    }

}
