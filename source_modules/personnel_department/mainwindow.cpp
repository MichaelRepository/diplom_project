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

    /// окно фильтра
    tablefilter = new MyFilterForm(this);
    tablefilter->setWindowFlags(Qt::Dialog);

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

    /// создание контекстного меню таблицы
    Table_record_edit   = new QAction(QIcon(":/svg/edit-icon.svg"), "Изменить запись", this);
    Table_record_add    = new QAction(QIcon(":/svg/plus-icon.svg"), "Добавить запись", this);
    Table_record_remove = new QAction(QIcon(":/svg/minus-icon.svg"),"Удалить записи",  this);
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


/// ВАЖНО - установка глобальной таблицы
    dlgrecordedit   = new DialogEditRecord(connectionname, this);
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
        Status_search_bt->setMenu(search_menu_for_spec);
        break;
    case GROUP:
        ui->Switch_table_group_button->setChecked(true);
        ui->To_group_button->setEnabled(false);
        ui->Change_state_button->setEnabled(false);
        ui->Change_course_button->setEnabled(true);
        Status_label_curtable->setText("Таблица: Группа ");
        Status_search_bt->setMenu(search_menu_for_group);
        break;
    case STUDENT:
        ui->Switch_table_stud_button->setChecked(true);
        ui->To_group_button->setEnabled(true);
        ui->Change_state_button->setEnabled(true);
        ui->Change_course_button->setEnabled(false);
        Status_label_curtable->setText("Таблица: Студент ");
        Status_search_bt->setMenu(search_menu_for_stud);
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
        if(!refresh_table(specialitytable) ) return ;
        globaltable = specialitytable;
        break;
    case GROUP:
        if(!refresh_table(grouptable) ) return ;
        globaltable = grouptable;
        break;
    case STUDENT:
        if(!refresh_table(studenttable) ) return ;
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

    globalsubtable = 0;
    refresh_menu();
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
    if(!index.isValid()) return ;
    globaltable->setCurrentRow(index.row());
    if(!globaltable->displayedFieldIsLink(index.column()) ) return;
    MyLink * link = globaltable->getdisplayedLink(index.column() );
    if(!link->table_1) return;
    QVariant key0_1val = globaltable->getFieldValue(link->key0_1);
    if(!key0_1val.isValid()) return;
    /// установить глобальную субтаблицу
    globalsubtable = link->table_1;
    globalsubtable->createQuickFilter(link->key1_0, "=", key0_1val.toString());
    globalsubtable->setFilterActivity(true);
    globalsubtable->setCurrentRow(-1);
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

void MainWindow::remove_records(MyTable *table)                                 /// удалить записи
{
    if(!table->removeRecord(table->getCurrentRowIndex() ))
    {
        QMessageBox messbox;
        messbox.setWindowTitle("Ошибка");
        messbox.setText("Невозможно удалить запись.");
        messbox.setIcon(QMessageBox::Critical);
        messbox.addButton(QMessageBox::Ok);
        messbox.setButtonText(QMessageBox::Ok, "Ясно");
        messbox.exec();
    }
    refresh_table(table);
}

void MainWindow::edit_records(MyTable *table)                                   /// изменить записи
{
    dlgrecordedit->setRecord(table->getRecord(table->getCurrentRowIndex() ) );
    if(dlgrecordedit->exec() == 1)
    {
        if(!table->setUpdateDataFields())
        {
            QMessageBox messbox;
            messbox.setWindowTitle("Ошибка");
            messbox.setText("Невозможно обновить запись.");
            messbox.setIcon(QMessageBox::Critical);
            messbox.addButton(QMessageBox::Ok);
            messbox.setButtonText(QMessageBox::Ok, "Ясно");
            messbox.exec();
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
    }
    tablemodel   ->refresh();
    subtablemodel->refresh();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
    return res;
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
    edit_records(globaltable);
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
    remove_records(globaltable);
}

void MainWindow::on_Filter_checked_button_clicked()
{
    bool oldstate = globaltable->isFiltered();
    globaltable->setFilterActivity(!oldstate);
    bool newstate = globaltable->isFiltered();
    if(newstate != oldstate)
    {
        newstate &= refresh_table(globaltable);
        ui->Filter_checked_button->setChecked(newstate);
    }
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
    edit_records(globalsubtable);
    subtablewidget->show();
}

void MainWindow::subtableRecordAdd()
{
    if(globalsubtable->getCurrentRowIndex() == -1) return;
    add_new_record(globalsubtable);
    subtablewidget->show();
}

void MainWindow::subtableRecordRemove()
{
    if(globalsubtable->getCurrentRowIndex() == -1) return;
    remove_records(globalsubtable);
}

void MainWindow::subtableRowSelected(int row)
{
    globalsubtable->setCurrentRow(row);
}

void MainWindow::on_Master_filter_button_clicked()
{
    tablefilter->setTable(globaltable);
    tablefilter->show();
}
