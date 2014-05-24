#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    userid = -1;
    connectionname = "wowdb";
    globaltable    = 0;
    globalsubtable = 0;

    /// убрать знак вопроса в окне авторизации
    Qt::WindowFlags authdlgflags = authorizedlg.windowFlags();
    authdlgflags = authdlgflags &(~Qt::WindowContextHelpButtonHint);
    authorizedlg.setWindowFlags(authdlgflags);

    /// окно фильтра
    tablefilter = new MyFilterForm(this);
    Qt::WindowFlags flags = Qt::Window;
    flags = flags | Qt::WindowCloseButtonHint ;
    tablefilter->setWindowFlags(flags);

    connect(tablefilter, &MyFilterForm::filterready,
            this,        &MainWindow::setGlobalTableFiltered);

    /// создание окна для отображения подтаблиц
    subtablewidget = new SubTableWidget(this);                                  /// установка флага - "контекстное меню", для диалога субтаблиц
    subtablewidget->setWindowFlags(Qt::Popup);
    subtablewidget->setTitleText("");
    /// подключение слотов главного окна к сигналам окна для отображения подтаблиц
    connect(subtablewidget, &SubTableWidget::editButtonClicked,
            this,           &MainWindow::subtableRecordEdit);
    connect(subtablewidget, &SubTableWidget::addButtonClicked,
            this,           &MainWindow::subtableRecordAdd);
    connect(subtablewidget, &SubTableWidget::removeButtonClicked,
            this,           &MainWindow::subtableRecordRemove);
    connect(subtablewidget, &SubTableWidget::newrowselected,
            this,           &MainWindow::subtableRowSelected);

    /// создание элементов статус бара
    Status_label_curtable       = new QLabel(this);
    Status_label_count_rows     = new QLabel(this);
    Status_label_count_selected = new QLabel(this);
    Status_search_frame         = new QFrame(this);
    Status_search_edit          = new QLineEdit  (Status_search_frame);
    Status_search_bt            = new QToolButton(Status_search_frame);
    Status_search_bt->setPopupMode(QToolButton::InstantPopup);

    search_menu_for_spec        = new QMenu(this);
    search_menu_for_group       = new QMenu(this);
    search_menu_for_stud        = new QMenu(this);

    connect(Status_search_edit, &QLineEdit::textEdited,
            this,               &MainWindow::searchStart);
    connect(Status_search_bt,   &QToolButton::triggered,
            this,               &MainWindow::searchBtMenuTriggered);

    ui->statusBar->addPermanentWidget(Status_search_frame);
    ui->statusBar->addPermanentWidget(Status_label_curtable);
    ui->statusBar->addPermanentWidget(Status_label_count_rows);
    ui->statusBar->addPermanentWidget(Status_label_count_selected,1);
    ui->statusBar->setStyleSheet("QStatusBar::item{border:none;}");
    /// компоновка элементов фрейма Status_search_frame
    QGridLayout* layout = new QGridLayout(Status_search_frame);
    Status_search_frame->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(Status_search_edit,0,0,1,1);
    Status_search_frame->setMinimumWidth(170);
    Status_search_frame->setMaximumHeight(20);
    Status_search_frame->setStyleSheet  ("QToolButton, QFrame{margin-left:5px}");
    Status_search_bt->setAutoRaise(true);
    Status_search_bt->setIcon(QIcon(":/png/search-icon.png") );
    Status_search_bt->setToolButtonStyle(Qt::ToolButtonIconOnly);
    Status_search_bt->setMaximumWidth(25);
    Status_search_bt->setMinimumWidth(25);
    Status_search_bt->setMaximumHeight(19);
    Status_search_edit->setStyleSheet("QLineEdit{padding-left:20px;}");

    header = new QSpreadsheetHeaderView(Qt::Horizontal, this);                  /// установка собственного заголовка для столбцов
    header->setHighlightSections(true);

    connect(header, &QSpreadsheetHeaderView::sortUp,
            this,   &MainWindow::sortUP);
    connect(header, &QSpreadsheetHeaderView::sortDown,
            this,   &MainWindow::sortDown);
    connect(header, &QSpreadsheetHeaderView::orderClear,
            this,   &MainWindow::clearOrders);

    /// создание контекстного меню таблицы
    Table_refresh       = new QAction(QIcon(":/svg/refresh-icon.svg"),      "Обновить данные",     this);
    Table_record_edit   = new QAction(QIcon(":/svg/edit-icon.svg"),         "Изменить запись",     this);
    Table_record_add    = new QAction(QIcon(":/svg/plus-icon.svg"),         "Добавить запись",     this);
    Table_record_remove = new QAction(QIcon(":/svg/minus-icon.svg"),        "Удалить записи",      this);
    Table_set_filtered  = new QAction(QIcon(":/svg/filter-icon.svg"),       "Активировать фильтр", this);
    Table_master_filter = new QAction(QIcon(":/svg/master_filter-icon.svg"),"Мастер фильтр",       this);

    connect(Table_refresh,       &QAction   ::triggered,
            this,                &MainWindow::on_Refresh_button1_clicked);
    connect(Table_record_edit,   &QAction   ::triggered,
            this,                &MainWindow::on_Edit_button1_clicked);
    connect(Table_record_add ,   &QAction   ::triggered,
            this,                &MainWindow::on_Add_button1_clicked);
    connect(Table_record_remove, &QAction   ::triggered,
            this,                &MainWindow::on_Delete_button1_clicked);
    connect(Table_set_filtered,  &QAction   ::triggered,
            this,                &MainWindow::on_Filter_checked_button_clicked);
    connect(Table_master_filter, &QAction   ::triggered,
            this,                &MainWindow::on_Master_filter_button_clicked);

    ui->tableView->addAction(Table_refresh);
    ui->tableView->addAction(Table_record_edit);
    ui->tableView->addAction(Table_record_add);
    ui->tableView->addAction(Table_record_remove);
    ui->tableView->addAction(Table_master_filter);
    ui->tableView->addAction(Table_set_filtered);
    ui->tableView->setContextMenuPolicy(Qt::ActionsContextMenu);                /// установка типа меню - отображать список действий

    /// создание загрузчика параметров приложения
    setting = new QSettings("../myapp.ini",QSettings::IniFormat,this);

    /// подсказки
    ui->Edit_button1->                  setToolTip("Обновить данные записи");
    ui->Add_button1->                   setToolTip("Добавить новую запись в таблицу");
    ui->Delete_button1->                setToolTip("Удалить выделенные записи из таблицы");
    ui->Refresh_button1->               setToolTip("Обновить (перезапросить) данные в таблице");
    ui->Master_filter_button->          setToolTip("Вызвать диалог для построения фильтра");
    ui->Filter_selected_button->        setToolTip("Отобрать выделенные записи");
    ui->Filter_unselected_button->      setToolTip("Отобрать невыделенные записи");
    ui->Filter_checked_button->         setToolTip("Включить/выключить фильтрацию записей");
    ui->Switch_table_spec_button->      setToolTip("Переключиться на таблицу [Специальности]");
    ui->Switch_table_group_button->     setToolTip("Переключиться на таблицу [Группы]");
    ui->Switch_table_stud_button->      setToolTip("Переключиться на таблицу [Студенты]");
    Status_search_bt->                  setToolTip("Выбрать поле для поиска");
    ui->Create_Report_bt1->             setToolTip("Создать отчет");
    ui->Refresh_button2->               setToolTip("Обновить таблицу шаблонов");
    ui->Export_button1->                setToolTip("Экспорт данных таблицы в файл .odt");
}

MainWindow::~MainWindow()
{
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    db.close();

    delete tablemodel;
    delete subtablemodel;

    delete metadatasource;

    delete specialitytable;
    delete grouptable;
    delete studenttable;
    delete citizenshiptable;
    delete residencetable;

    delete ui;
}

void MainWindow::initTables()
{
    qDebug() << "tables start init" << QDateTime::currentDateTimeUtc().toString("hh:mm:ss zzz");
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
    residencetable->appendField("region",         "residence", true, true);
    residencetable->appendField("area",           "residence", true, true);
    residencetable->appendField("city",           "residence", true, true);
    residencetable->appendField("locality",       "residence", true, true);
    residencetable->appendField("street",         "residence", true, true);
    residencetable->appendField("house",          "residence", true, true);
    residencetable->appendField("apartment",      "residence", true, true);
    residencetable->appendField("registered",     "residence", true, true);
    residencetable->appendField("subject",        "residence", false, false);

/// подготовить таблицу - специальности
    specialitytable->appendField("idspeciality",     "speciality", true, true);
    specialitytable->appendField("abbreviation",     "speciality", true, true);
    specialitytable->appendField("specialityname",   "speciality", true, true);
    specialitytable->appendField("periodeducation",  "speciality", true, true);
    specialitytable->appendField("specialization",   "speciality", true, true);

/// подготовить таблицу - группы
    grouptable->appendField("idgroup",      "groups", false, true);
    grouptable->appendField("speciality",   "groups", true,  true);
    grouptable->appendField("groupname",    "groups", true,  true);
    grouptable->appendField("yearformation","groups", true,  true);
    grouptable->appendField("form",         "groups", true,  true);
    grouptable->appendField("budget",       "groups", true,  true);
    grouptable->appendField("course",       "groups", true,  true);
    grouptable->appendField("basedon",      "groups", true,  true);

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

    studenttable->appendField("passnum",             "subject", true, true);
    studenttable->appendField("passseries",          "subject", true, true);
    studenttable->appendField("passcodedepartament", "subject", true, true);
    studenttable->appendField("passissued",          "subject", true, true);
    studenttable->appendField("passdateissue",       "subject", true, true);

    studenttable->appendLink ("Проживание", "idsubject", "subject","", residencetable);

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

/// формирование данных для поискового элемента
    QList<const MyField *> fields;

    fields = specialitytable->getDisplayedFields();
    prepareSearchData(fields, search_data_for_spec,  search_menu_for_spec);

    fields = grouptable->getDisplayedFields();
    prepareSearchData(fields, search_data_for_group, search_menu_for_group);

    fields = studenttable->getDisplayedFields();
    prepareSearchData(fields, search_data_for_stud,  search_menu_for_stud);

    current_search_key_for_spec  = "abbreviation";
    current_search_key_for_group = "groupname";
    current_search_key_for_stud  = "surname";

    search_menu_for_spec ->actions().at(1)->setChecked(true);
    search_menu_for_group->actions().at(2)->setChecked(true);
    search_menu_for_stud ->actions().at(2)->setChecked(true);

/// меню для дополнительных таблиц
    act_group_for_tables         = new QActionGroup(this);
    act_group_for_tables->setExclusive(true);

    act_citizen_table            = new QAction("Гражданство",       act_group_for_tables);
    act_school_table             = new QAction("Уч. заведения",     act_group_for_tables);
    act_typeducation_table       = new QAction("Тип образования",   act_group_for_tables);
    act_typeschool_table         = new QAction("Тип уч. заведений", act_group_for_tables);
    act_privilegescategory_table = new QAction("Категории льгот",   act_group_for_tables);

    act_citizen_table->             setCheckable(true);
    act_school_table->              setCheckable(true);
    act_typeducation_table->        setCheckable(true);
    act_typeschool_table->          setCheckable(true);
    act_privilegescategory_table->  setCheckable(true);

    act_citizen_table->             setData(QVariant(CITIZENSHIP) );
    act_school_table->              setData(QVariant(SCHOOL) );
    act_typeducation_table->        setData(QVariant(TYPEEDUCATION) );
    act_typeschool_table->          setData(QVariant(TYPESCHOOL));
    act_privilegescategory_table->  setData(QVariant(PRIVILEGESCATEGORY) );

    ui->Switch_table_addition_button->addAction(act_citizen_table);
    ui->Switch_table_addition_button->addAction(act_school_table);
    ui->Switch_table_addition_button->addAction(act_typeducation_table);
    ui->Switch_table_addition_button->addAction(act_typeschool_table);
    ui->Switch_table_addition_button->addAction(act_privilegescategory_table);
    ui->Switch_table_addition_button->setPopupMode(QToolButton::InstantPopup);

    connect(ui->Switch_table_addition_button, &QToolButton::triggered,
            this,                             &MainWindow:: set_additionally_table);

/// ВАЖНО - установка глобальной таблицы
    dlgrecordedit   = new DialogEditRecord(connectionname, this);
    Qt::WindowFlags flags = dlgrecordedit->windowFlags();
    flags = flags & (~Qt::WindowContextHelpButtonHint);
    dlgrecordedit->setWindowFlags(flags);
    tablemodel      = new MyTableModel(this);
    subtablemodel   = new MyTableModel(this);
    globaltable     = specialitytable;

    qDebug() << "tables end init:" << QDateTime::currentDateTimeUtc().toString("hh:mm:ss zzz");
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
        query0.prepare("SELECT iduser, type FROM operators WHERE login = :login "
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
            /// получение идентификаторов пользователя и группы
            userid  = query0.value(0).toInt();
            groupid = query0.value(1).toInt();
            /* КАЖДОЕ КЛИЕНТСКОЕ ПРИЛОЖЕНИЕ СООТВЕТСВУЕТ СВОЕЙ ГРУППЕ ПОЛЬЗОВАТЕЛЕЙ */
            if(groupid != 3)
            {
                QMessageBox messbox;
                messbox.setIcon(QMessageBox::Warning);
                messbox.setWindowTitle("Сообщение");
                messbox.setText("Доступ закрыт!\n"
                                "Вы не являетесь работником отдела кадров.");
                messbox.addButton(QMessageBox::Ok);
                messbox.setButtonText(QMessageBox::Ok,"ясно");
                messbox.exec();
            }
            else
                break;
        }
    }
    if(userid == -1) exit(0);

    initTables();                                                               /// ИНИЦИАЛИЗАЦИЯ ТАБЛИЦ
    set_current_table(SPECIALITY);                                              /// установить текущую таблицу

    /// получение шаблонов отчетов
    reportstablemodel = new QSqlTableModel(this);                               /// модель данных для оторажения таблицы шаблонов
    getReportTemplatesTable();

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
        Status_label_curtable->setText("Таблица: Специальность ");
        Status_search_bt->setMenu(search_menu_for_spec);
        break;
    case GROUP:
        Status_label_curtable->setText("Таблица: Группа ");
        Status_search_bt->setMenu(search_menu_for_group);
        break;
    case STUDENT:
        Status_label_curtable->setText("Таблица: Студент ");
        Status_search_bt->setMenu(search_menu_for_stud);
        break;
    case CITIZENSHIP:
        Status_label_curtable->setText("Таблица: Гражданство ");
        break;
    case PRIVILEGESCATEGORY:
        Status_label_curtable->setText("Таблица: Льготы ");
        break;
    case SCHOOL:
        Status_label_curtable->setText("Таблица: Уч. заведения ");
        break;
    case TYPESCHOOL:
        Status_label_curtable->setText("Таблица: Типы уч. заведений ");
        break;
    case TYPEEDUCATION:
        Status_label_curtable->setText("Таблица: Тип образования ");
        break;
    }
    /// определить число выделенных записей активной таблицы
    int countselectedrows = ui->tableView->selectionModel()->selectedRows().size();
    if(countselectedrows == 1) globaltable->setCurrentRow(ui->tableView->selectionModel()->selectedRows().first().row());
    /// переключить состояние кнопок и элементов контекстного меню, зависимых от числа выделенных записей
    ui->Edit_button1->    setEnabled(countselectedrows > 0);
    ui->Delete_button1->  setEnabled(countselectedrows > 0);
    Table_record_edit->   setEnabled(countselectedrows > 0);
    Table_record_remove-> setEnabled(countselectedrows > 0);
    /// обновить надписи в статус баре
    Status_label_count_rows->setText    (" Число записей: "+ QString::number(globaltable->getRecordsCount())+" ");
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));

    ui->Filter_checked_button->setChecked(globaltable->isFiltered());
    if(globaltable->isFiltered())
        Table_set_filtered->setText("Деактивировать фильтр");
    else
        Table_set_filtered->setText("Активировать фильтр");
}

void MainWindow::set_current_table(Tables table)
{
    /// формирование запросов, относительно текущей таблицы
    switch(table){
    case SPECIALITY:
        ui->Switch_table_spec_button->setChecked(true);
        if(!refresh_table(specialitytable) ) return ;
        globaltable = specialitytable;
        break;
    case GROUP:
        ui->Switch_table_group_button->setChecked(true);
        if(!refresh_table(grouptable) ) return ;
        globaltable = grouptable;
        break;
    case STUDENT:
        ui->Switch_table_stud_button->setChecked(true);
        if(!refresh_table(studenttable) ) return ;
        globaltable = studenttable;
        break;
    case CITIZENSHIP:
        ui->Switch_table_addition_button->setChecked(true);
        if(!citizenshiptable->updateData() ) return ;
        globaltable = citizenshiptable;
        break;
    case PRIVILEGESCATEGORY:
        ui->Switch_table_addition_button->setChecked(true);
        return ;
        break;
    case SCHOOL:
        ui->Switch_table_addition_button->setChecked(true);
        return ;
        break;
    case TYPESCHOOL:
        ui->Switch_table_addition_button->setChecked(true);
        return ;
        break;
    case TYPEEDUCATION:
        ui->Switch_table_addition_button->setChecked(true);
        return ;
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

    tablefilter->setTable(globaltable);

    globalsubtable = 0;
    refresh_table(globaltable);
    refresh_menu();
}

void MainWindow::set_additionally_table(QAction *action)
{
    ui->Switch_table_addition_button->setText(action->text() );
    ui->Switch_table_addition_button->setChecked(true);
    Tables table = Tables(action->data().toInt() );
    set_current_table(table);
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid() ||  currenttable == STUDENT) return ;
    Tables nexttable;

    QModelIndexList list = ui->tableView->selectionModel()->selectedRows();
    if(!list.count()) return ;
    int row = list.first().row();
    globaltable->setCurrentRow(row);

    switch(currenttable){
    case SPECIALITY:{
        nexttable = GROUP;
        int currecord = specialitytable->getCurrentRowIndex();
        QString filtervalue = specialitytable->displayFieldValue(currecord,"idspeciality").toString();
        grouptable->createQuickFilter("speciality", "=", filtervalue);
        grouptable->setFilterActivity(true);
        break;
    }
    case GROUP:{
        nexttable = STUDENT;
        int currecord = grouptable->getCurrentRowIndex();
        QString filtervalue = grouptable->displayFieldValue(currecord,"idgroup").toString();
        studenttable->createQuickFilter("studentgroup", "=", filtervalue);
        studenttable->setFilterActivity(true);
        break;
    }
    }
    set_current_table(nexttable);                                               /// переключение таблиц
}

void MainWindow::on_Switch_table_spec_button_clicked()
{
    set_current_table(SPECIALITY);
}

void MainWindow::on_Switch_table_group_button_clicked()
{
    set_current_table(GROUP);
}

void MainWindow::on_Switch_table_stud_button_clicked()
{
    set_current_table(STUDENT);
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    if(!index.isValid()) return ;
    globaltable->setCurrentRow(index.row());
    if(!globaltable->displayedFieldIsLink(index.column()) ) return;
    MyLink * link = globaltable->getdisplayedLink(index.column() );
    if(!link->table_1) return;
    QVariant key0_1val = globaltable->getFieldValue(link->key0_1);
    if(!key0_1val.isValid()) return;
    /// установить глобальную субтаблицу
    globalsubtable = link->table_1;
    globalsubtable->createQuickFilter(link->key1_0, "=", key0_1val.toString() );
    globalsubtable->setFilterActivity(true);
    globalsubtable->setCurrentRow(-1);

    //!!! автоподставновка занчений красивое решение задачи добавления новой записи в слинкованной таблице:)
    globalsubtable->appendAutoValue(link->key1_0, key0_1val);

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

    subtablemodel->setTableObject(globalsubtable);
    if(!refresh_table(globalsubtable) ) return;

    subtablewidget->setDisplayMode(true,true,true);
    subtablewidget->setTableModel(subtablemodel);
    subtablewidget->move(pos);
    subtablewidget->show();
}

void MainWindow::tableView_items_selected()
{
    refresh_menu();
}

void MainWindow::add_new_record(MyTable *table)                                 /// добавить запись
{
    dlgrecordedit->setRecord(table->getEmptyRecordForInsert() );
    if(dlgrecordedit->exec() == 1)
    {
        QMessageBox mess;
        mess.setWindowTitle("Подтвердить действие.");
        mess.setIcon(QMessageBox::Warning);
        mess.setText("Желаете применить изменения?");
        mess.addButton(QMessageBox::Cancel);
        mess.addButton(QMessageBox::Ok);
        mess.setButtonText(QMessageBox::Ok, "Применить");
        mess.setButtonText(QMessageBox::Cancel,"Отмена");
        if(mess.exec() == QMessageBox::Cancel ) return ;

        if(!table->setInsertDataFields())
        {
            QMessageBox messbox;
            messbox.setWindowTitle("Ошибка");
            messbox.setText("Невозможно добавить запись.");
            messbox.setIcon(QMessageBox::Critical);
            messbox.addButton(QMessageBox::Ok);
            messbox.setButtonText(QMessageBox::Ok, "Ясно");
            messbox.exec();
        }
        refresh_table(table);
    }
}

void MainWindow::remove_records(MyTable *table, bool assubtable)                /// удалить записи
{
    QList<int> rows;
    if(assubtable)
        rows = subtablewidget->selectedRows();
    else
        rows = this->selectedRows();

    QMessageBox mess;
    mess.setIcon(QMessageBox::Warning);
    mess.setWindowTitle("Подтвердить удаление.");
    mess.setText("Желаете удалить выбранные записи?\n"
                 "Число удаляемых записей(строк): "+
                 QString::number(rows.size() ) );
    mess.addButton(QMessageBox::Ok);
    mess.addButton(QMessageBox::Cancel);
    mess.setButtonText(QMessageBox::Cancel,"Отмена");
    mess.setButtonText(QMessageBox::Ok, "Удалить");
    if(mess.exec() == QMessageBox::Cancel ) return ;

    if(!table->removeRecords(rows) )
    {
        QMessageBox messbox;
        messbox.setWindowTitle("Ошибка");
        messbox.setText("Невозможно удалить записи");
        messbox.setIcon(QMessageBox::Critical);
        messbox.addButton(QMessageBox::Ok);
        messbox.setButtonText(QMessageBox::Ok, "Ясно");
        messbox.exec();

        if(table->lastSqlError().isValid())
        {
            dbmessdlg.showdbmess(table->lastSqlError());
        }
    }
    refresh_table(table);
}

void MainWindow::edit_records(MyTable *table, bool assubtable)                  /// изменить записи
{
    QList<int> rows;
    if(assubtable)
        rows = subtablewidget->selectedRows();
    else
        rows = this->selectedRows();
    /// два режима редактирования!
    /// 1. редактирование одной записи
    /// 2. редактирование группы записей
    if(rows.size() > 1)
        dlgrecordedit->setRecord(table->getEmptyRecordForInsert() );
    else
        dlgrecordedit->setRecord(table->getRecord(table->getCurrentRowIndex() ) );

    if(dlgrecordedit->exec() == 1)
    {
        QMessageBox mess;
        mess.setWindowTitle("Применить изменения");
        mess.setIcon(QMessageBox::Warning);
        mess.setText("Желаете применить изменения?\n"
                     "Количество изменяемых записей(строк):"+
                     QString::number(rows.size() ) );
        mess.addButton(QMessageBox::Ok);
        mess.addButton(QMessageBox::Cancel);
        mess.setButtonText(QMessageBox::Ok, "Применить");
        mess.setButtonText(QMessageBox::Cancel,"Отмена");
        if(mess.exec() == QMessageBox::Cancel ) return  ;

        bool result;
        if(rows.size() > 1)
            result = table->setUpdateGroupOfRecords(rows);
        else
            result = table->setUpdateDataFields();

        if(!result)
        {
            QMessageBox messbox;
            messbox.setWindowTitle("Ошибка");
            messbox.setText("Невозможно обновить записи.");
            messbox.setIcon(QMessageBox::Critical);
            messbox.addButton(QMessageBox::Ok);
            messbox.setButtonText(QMessageBox::Ok, "Ясно");
            messbox.exec();

            if(table->lastSqlError().isValid())
            {
                dbmessdlg.showdbmess(table->lastSqlError());
            }

        }
        refresh_table(table);
    }
}

bool MainWindow::refresh_table(MyTable *table)
{
    bool res = table->updateData();
    if(!res)
    {
        QMessageBox messbox;
        messbox.setWindowTitle("Ошибка");
        messbox.setIcon(QMessageBox::Critical);
        messbox.setText("Произошла ошибка.\n"
                        "Данные таблицы не получены.");
        messbox.addButton(QMessageBox::Ok);
        messbox.setButtonText(QMessageBox::Ok, "Ясно");
        messbox.exec();

        if(table->lastSqlError().isValid())
        {
            dbmessdlg.showdbmess(table->lastSqlError());
        }
    }

    ui->tableView->clearSelection();
    tablemodel   ->refresh();
    subtablemodel->refresh();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    header->setCustomOrderData(globaltable->getCustomOrdersData() );

    return res;
}

void MainWindow::getReportTemplatesTable()
{
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.isOpen())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
    }
    QSqlQuery query(db);
    query.prepare("SELECT reports.idreport, reports.repname, reports.repinfo "
                  "FROM reports, usersgroup "
                  "WHERE usersgroup.idgroup = reports.usergroup AND "
                  "usersgroup.idgroup = ?");
    query.bindValue(0, groupid);

    if(!query.exec() )
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(query.lastError());
    }
    else
    {
        reportstablemodel->setQuery(query);
        ui->reportsTableView->setModel(reportstablemodel);
        ui->reportsTableView->horizontalHeader()->setFocusPolicy(Qt::NoFocus);

        ui->reportsTableView->model()->setHeaderData(0,Qt::Horizontal,QVariant("#"));
        ui->reportsTableView->model()->setHeaderData(1,Qt::Horizontal,QVariant("Название"));
        ui->reportsTableView->model()->setHeaderData(2,Qt::Horizontal,QVariant("Описание"));

        ui->reportsTableView->resizeColumnsToContents();
        ui->reportsTableView->resizeRowsToContents();

        QItemSelectionModel *sm = ui->reportsTableView->selectionModel();
        connect(sm,   &QItemSelectionModel::currentChanged,
                this, &MainWindow::         templatSelected);

        if(reportstablemodel->query().size() > 0)
        {
            sm->setCurrentIndex(reportstablemodel->index(0,0),
                                QItemSelectionModel::ClearAndSelect |
                                QItemSelectionModel::Rows);
        }
    }
}

void MainWindow::templatSelected(const QModelIndex &current,
                                 const QModelIndex &previous)
{
    int row = current.row();
    int id  = current.model()->index(row,0).data().toInt();
    QString name = current.model()->index(row,1).data().toString();
    QString inf  = current.model()->index(row,2).data().toString();
    ui->RepName->setText(name);
    ui->RepInfo->setPlainText(inf);

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    QSqlQuery query(db);
    query.prepare("SELECT repico FROM reports WHERE idreport = ?");
    query.bindValue(0,QVariant(id));
    if(query.exec() )
    {
        query.first();
        QPixmap map;
        QByteArray data = query.value(0).toByteArray();
        map.loadFromData(data);
        map = map.scaled(128,128, Qt::KeepAspectRatio,Qt::SmoothTransformation);
        ui->thumbLabel->setPixmap(map);
        ui->thumbLabel->setAlignment(Qt::AlignCenter);
    }
}

QList<int> MainWindow::selectedRows() const
{
    QList<int> rows;
    QModelIndexList items = ui->tableView->selectionModel()->selectedRows(0);
    QModelIndexList::const_iterator itri;
    for(itri = items.begin(); itri != items.end(); ++itri) rows << (*itri).row();
    return rows;
}

void MainWindow::createReport()
{
    QModelIndex index = ui->reportsTableView->currentIndex();
    if(!index.isValid()) return;
    int row = index.row();
    int id  = index.model()->index(row,0).data().toInt();

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    QSqlQuery query(db);
    query.prepare("SELECT repdata FROM reports WHERE idreport = ?");
    query.bindValue(0, QVariant(id));
    if(!query.exec())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(query.lastError());
        return ;
    }
    query.first();

    QByteArray odtdata = query.value(0).toByteArray();

    MyDocumentODF report;
    report.setConnectionName(connectionname);
    if(!report.readDocumentData(odtdata) )
    {
        if(report.isError())
        {
            QMessageBox mess;
            mess.setWindowTitle("Ошибка построения отчета");
            mess.setText("Произошла ошибка в процессе построения отчета.");
            mess.setIcon(QMessageBox::Critical);
            mess.addButton(QMessageBox::Ok);
            mess.setButtonText(QMessageBox::Ok, "Ясно");
            mess.exec();
        }
        else
        {
            QMessageBox mess;
            mess.setWindowTitle("Отмена");
            mess.setText("Действие отменено. Отчет не создан.");
            mess.setIcon(QMessageBox::Information);
            mess.addButton(QMessageBox::Ok);
            mess.setButtonText(QMessageBox::Ok, "Ясно");
            mess.exec();
        }
        return ;
    }

    QFileDialog filedlg;
    QString newfile = filedlg.getSaveFileName(0,"Сохранение файла","","ODT files (*.odt)");
    this->activateWindow();
    if(newfile.isEmpty() ) return;
    report.saveDocumentCopy(newfile);
    QDesktopServices serv;

    if(!serv.openUrl(QUrl::fromLocalFile(newfile)) )
    {
        QMessageBox mess;
        mess.setWindowTitle("Ошибка построения отчета");
        mess.setText("Произошла ошибка в процессе построения отчета.");
        mess.setIcon(QMessageBox::Critical);
        mess.addButton(QMessageBox::Ok);
        mess.setButtonText(QMessageBox::Ok, "Ясно");
        mess.exec();
        return ;
    }
    this->activateWindow();
}

void MainWindow::exportInDoc()
{
    QList<int> columns;
    QStringList fieldsname;
    for(int i = 0; i < header->count(); ++i)
    {
        if(!header->isSectionHidden(i) )
        {
            columns << i;
            fieldsname << tablemodel->headerData(i,Qt::Horizontal).toString();
        }
    }
    columns = globaltable->fieldsIndexOfColumns(columns);
    QSqlQuery querydata = globaltable->getTableQueryData();

    MyDocumentODF report;
    report.setConnectionName(connectionname);

    if(!report.createDocumentFrom(querydata, fieldsname, columns ) )
    {
        if(report.isError())
        {
            QMessageBox mess;
            mess.setWindowTitle("Ошибка построения отчета");
            mess.setText("Произошла ошибка в процессе построения отчета.");
            mess.setIcon(QMessageBox::Critical);
            mess.addButton(QMessageBox::Ok);
            mess.setButtonText(QMessageBox::Ok, "Ясно");
            mess.exec();
        }
        else
        {
            QMessageBox mess;
            mess.setWindowTitle("Отмена");
            mess.setText("Действие отменено. Отчет не создан.");
            mess.setIcon(QMessageBox::Information);
            mess.addButton(QMessageBox::Ok);
            mess.setButtonText(QMessageBox::Ok, "Ясно");
            mess.exec();
        }
        return ;
    }

    QFileDialog filedlg;
    QString newfile = filedlg.getSaveFileName(0,"Сохранение файла","","ODT files (*.odt)");
    this->activateWindow();
    if(newfile.isEmpty() ) return;
    report.saveDocumentCopy(newfile);
    QDesktopServices serv;

    if(!serv.openUrl(QUrl::fromLocalFile(newfile)) )
    {
        QMessageBox mess;
        mess.setWindowTitle("Ошибка построения отчета");
        mess.setText("Произошла ошибка в процессе построения отчета.");
        mess.setIcon(QMessageBox::Critical);
        mess.addButton(QMessageBox::Ok);
        mess.setButtonText(QMessageBox::Ok, "Ясно");
        mess.exec();
        return ;
    }
    this->activateWindow();
}

void MainWindow::createFilterForSelectedRecords()
{
    QModelIndexList items = ui->tableView->selectionModel()->selectedRows(0);
    if(items.size() <= 0 ) return ;
    QList<int> rows;
    QModelIndexList::const_iterator itri;
    for(itri = items.begin(); itri != items.end(); ++itri) rows << (*itri).row();
    globaltable->createFilterForRecords(rows);
    setGlobalTableFiltered();
}

void MainWindow::createFilterForUnSelectedRecords()
{
    QModelIndexList items = ui->tableView->selectionModel()->selectedRows(0);
    if(items.size() <= 0 ) return ;
    QList<int> selectedrows;
    QModelIndexList::const_iterator itri;
    for(itri = items.begin(); itri != items.end(); ++itri) selectedrows << (*itri).row();

    QList<int> unselectedrows;
    for(int i = 0; i < globaltable->getRecordsCount(); ++i)
        if(!selectedrows.contains(i)) unselectedrows << i;
    globaltable->createFilterForRecords(unselectedrows);
    setGlobalTableFiltered();
}

void MainWindow::prepareSearchData(const QList<const MyField *> &fields,
                                   QMap<QString, QString> &data,
                                   QMenu* menu)
{
    QList<const MyField *>::const_iterator itr;
    QActionGroup *actionGroup = new QActionGroup(menu);
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        QString alter = (*itr)->altername;
        if(alter.isEmpty()) alter = (*itr)->name;
        data.insert(alter, (*itr)->name);
        QAction* action = new QAction(alter, actionGroup);
        actionGroup->addAction(action);
        menu->addAction(action);
        action->setCheckable(true);
    }

}

void MainWindow::on_Edit_button1_clicked()
{
    edit_records(globaltable, false);
}

void MainWindow::on_Refresh_button1_clicked()
{
    refresh_table(globaltable);
}

void MainWindow::on_Add_button1_clicked()
{
    add_new_record(globaltable);
}

void MainWindow::on_Delete_button1_clicked()
{
    remove_records(globaltable, false);
}

void MainWindow::on_Filter_checked_button_clicked()
{
    bool oldstate = globaltable->isFiltered();
    globaltable->setFilterActivity(!oldstate);
    bool newstate = globaltable->isFiltered();
    if(newstate != oldstate)
        newstate &= refresh_table(globaltable);
    if(newstate) Table_set_filtered->setText("Деактивировать фильтр");
    else         Table_set_filtered->setText("Активировать фильтр");
    ui->Filter_checked_button->setChecked(newstate);
}

void MainWindow::searchStart()
{
    QString findvalue = Status_search_edit->text();
    QItemSelectionModel* selectionmodel = ui->tableView->selectionModel();
    QAbstractItemModel*  model          = ui->tableView->model();
    if(findvalue.size() < 1) {selectionmodel->clearSelection(); return;}
    QString fieldname;
    switch(currenttable){
    case SPECIALITY:
        fieldname = current_search_key_for_spec;
        break;
    case GROUP:
        fieldname = current_search_key_for_group;
        break;
    case STUDENT:
        fieldname = current_search_key_for_stud;
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

void MainWindow::searchBtMenuTriggered(QAction* action)
{
    QString fieldaltername = action->text();
    action->setChecked(true);

    switch(currenttable){
    case SPECIALITY:
        current_search_key_for_spec  = search_data_for_spec[fieldaltername];
        break;
    case GROUP:
        current_search_key_for_group = search_data_for_group[fieldaltername];
        break;
    case STUDENT:
        current_search_key_for_stud  = search_data_for_stud[fieldaltername];
        break;
    }
}

void MainWindow::subtableRecordEdit()
{
    if(globalsubtable->getCurrentRowIndex() == -1) return;
    edit_records(globalsubtable, true);
    subtablewidget->show();
}

void MainWindow::subtableRecordAdd()
{
    add_new_record(globalsubtable);
    subtablewidget->show();
}

void MainWindow::subtableRecordRemove()
{
    if(globalsubtable->getCurrentRowIndex() == -1) return;
    remove_records(globalsubtable, true);
    subtablewidget->show();
}

void MainWindow::subtableRowSelected(int row)
{
    globalsubtable->setCurrentRow(row);
}

void MainWindow::on_Master_filter_button_clicked()
{
    tablefilter->show();
}

void MainWindow::setGlobalTableFiltered()
{
    globaltable->setFilterActivity(true);
    refresh_table(globaltable);
    ui->Filter_checked_button->setChecked(globaltable->isFiltered());
    if(globaltable->isFiltered())
        Table_set_filtered->setText("Деактивировать фильтр");
    else
        Table_set_filtered->setText("Активировать фильтр");
}

void MainWindow::on_Filter_selected_button_clicked()
{
    createFilterForSelectedRecords();
}

void MainWindow::on_Filter_unselected_button_clicked()
{
    createFilterForUnSelectedRecords();
}

void MainWindow::sortUP(int column, QString name)
{
    globaltable->setCustomOrderData(column, "ASC");
    refresh_table(globaltable);
}

void MainWindow::sortDown(int column, QString name)
{
    globaltable->setCustomOrderData(column, "DESC");
    refresh_table(globaltable);
}

void MainWindow::clearOrders()
{
    globaltable->clearCustomOrdersData();
    refresh_table(globaltable);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 2)
    {
        Status_search_frame->setVisible         (false);
        Status_label_curtable->setVisible       (false);
        Status_label_count_rows->setVisible     (false);
        Status_label_count_selected->setVisible (false);
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
    {
        Status_search_frame->setVisible         (true);
        Status_label_curtable->setVisible       (true);
        Status_label_count_rows->setVisible     (true);
        Status_label_count_selected->setVisible (true);
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_Refresh_button2_clicked()
{
    getReportTemplatesTable();
}

void MainWindow::on_Create_Report_bt1_clicked()
{
    createReport();
}

void MainWindow::on_Export_button1_clicked()
{    
    exportInDoc();
}
