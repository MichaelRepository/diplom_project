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

    //QApplication::processEvents();

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
        dbmessdlg.showdbmess(db.lastError());
        exit(0);
    }

/*
    // временн`ая заглушка. убрать!ё
    QTime time;
    time.start();

    for (int i = 0; i < 1000; ){
        splashwindow->showMessage("Loaded modules "+QString::number(i)+"%");
        QApplication::processEvents();
        if (time.elapsed() > 10) {
            time.start();
            ++i;
        }
    }
*/
    splashwindow->finish(&authorizedlg);
    /// создание окна для отображения подтаблиц
    subtabledlg = new SubTableDialog(connectionname, this);                        /// установка флага - "контекстное меню", для диалога субтаблиц
    subtabledlg->setWindowFlags(Qt::Popup);

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
    set_current_table(SPECIALITY, BUTTONMODE, 0);                               /// установить текущую таблицу

    QItemSelectionModel* selectionmodel = ui->tableView->selectionModel();      /// обработка выделения строк
    /*QObject::connect(selectionmodel,
                     SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                     this, SLOT (on_tableView_items_selected()) );*/

    QObject::connect (selectionmodel, &QItemSelectionModel::selectionChanged,
                      this,           &MainWindow::tableView_items_selected);

    show();
}

QString MainWindow::get_connect_name()
{
    return connectionname;
}

int MainWindow::get_selected_rows()
{
    return ui->tableView->selectionModel()->selectedRows().size();
}

void MainWindow::refresh_menu()
{
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
    Status_label_count_rows->setText(" Число записей: "+
                                     QString::number(query->size())+" ");
    int countselectedrows = ui->tableView->selectionModel()->selectedRows().size();
    Table_record_edit->setEnabled(countselectedrows == 1);
    Status_label_count_selected->setText("Выделено строк: "+QString::number(countselectedrows));
}

void MainWindow::set_current_table(Tables table, ModeSwitchingTable mode,
                                   int keyvalue)
{
    query->finish();                                                            /// сбросить предыдущий результат запроса
                                                                                /// формирование запросов, относительно текущей таблицы
    switch(table){
    case SPECIALITY:
        if(mode == BUTTONMODE)
            query->prepare("SELECT * FROM speciality ORDER BY idspeciality");
        else return ;
        break;
    case GROUP:
        if(mode == BUTTONMODE)
            query->prepare("SELECT * FROM groups ORDER BY idgroup");
        else{
            query->prepare("SELECT * FROM groups WHERE speciality = :id "
                           " ORDER BY idgroup");
            query->bindValue(":id",keyvalue);
        }
        break;
    case STUDENT:
        if(mode == BUTTONMODE)
            query->prepare(
                        "SELECT student.subject, student.numbertestbook, "
                                "subject.surname, subject.name, subject.patronymic, subject.sex, "
                                "groups.name, groups.course, groups.form, groups.budget, "
                                "subject.datebirth, subject.placebirth, "
                                "'...' AS citizenship, "
                                "'...' AS residence, "
                                "'...' AS passports, "
                                "'...' AS represent, "
                                "'...' AS education, "
                                "'...' AS privileges, "
                               /** "'...' AS progress, " **/
                                "'...' AS moreinf	 "
                        "FROM 	student "
                        "LEFT JOIN groups  ON (student.group = groups.idgroup) "
                        "LEFT JOIN subject ON (student.subject = subject.idsubject) "

                        );
        else{
            query->prepare("SELECT * FROM student INNER JOIN subject ON "
                           " student.subject = subject.idsubject AND "
                           " student.group = :id ORDER BY subject");
            query->bindValue(":id",keyvalue);           

        }
        break;
    }

    if(!query->exec())                                                          /// выполнить запрос
    {
        dbmessdlg.showdbmess(query->lastError());
        query->finish();
        return ;
    }
    currenttable = table;
    querymodel->setQuery(*query);                                               /// связать модель с запросом
                                                                                /// подпись заголовков таблицы
    switch(table){
    case SPECIALITY:
        ui->tableView->model()->setHeaderData(0, Qt::Horizontal, tr("Шифр"));
        ui->tableView->model()->setHeaderData(1, Qt::Horizontal, tr("Аббревиатура"));
        ui->tableView->model()->setHeaderData(2, Qt::Horizontal, tr("Наименование"));
        ui->tableView->model()->setHeaderData(3, Qt::Horizontal, tr("Период обучения"));
        ui->tableView->model()->setHeaderData(4, Qt::Horizontal, tr("Базируется на"));
        ui->tableView->model()->setHeaderData(5, Qt::Horizontal, tr("Специализация"));
        break;
    case GROUP:
        ui->tableView->model()->setHeaderData(0, Qt::Horizontal, tr("Номер"));
        ui->tableView->model()->setHeaderData(1, Qt::Horizontal, tr("Специальность"));
        ui->tableView->model()->setHeaderData(2, Qt::Horizontal, tr("Наименование"));
        ui->tableView->model()->setHeaderData(3, Qt::Horizontal, tr("Год формирования"));
        ui->tableView->model()->setHeaderData(4, Qt::Horizontal, tr("Форма обучения"));
        ui->tableView->model()->setHeaderData(5, Qt::Horizontal, tr("Бюджет"));
        break;
    case STUDENT:
        ui->tableView->model()->setHeaderData(0,    Qt::Horizontal, tr("#"));
        ui->tableView->model()->setHeaderData(1,    Qt::Horizontal, tr("Номер зачетки"));
        ui->tableView->model()->setHeaderData(2,    Qt::Horizontal, tr("Фамилия"));
        ui->tableView->model()->setHeaderData(3,    Qt::Horizontal, tr("Имя"));
        ui->tableView->model()->setHeaderData(4,    Qt::Horizontal, tr("Отчество"));
        ui->tableView->model()->setHeaderData(5,    Qt::Horizontal, tr("Пол"));
        ui->tableView->model()->setHeaderData(6,    Qt::Horizontal, tr("Группа"));
        ui->tableView->model()->setHeaderData(7,    Qt::Horizontal, tr("Курс"));
        ui->tableView->model()->setHeaderData(8,    Qt::Horizontal, tr("Форма обучения"));
        ui->tableView->model()->setHeaderData(9,    Qt::Horizontal, tr("Бюджет"));
        ui->tableView->model()->setHeaderData(10,   Qt::Horizontal, tr("Дата рождения"));
        ui->tableView->model()->setHeaderData(11,   Qt::Horizontal, tr("Место рождения"));
        ui->tableView->model()->setHeaderData(12,   Qt::Horizontal, tr("Гражданство"));
        ui->tableView->model()->setHeaderData(13,   Qt::Horizontal, tr("Проживание"));
        ui->tableView->model()->setHeaderData(14,   Qt::Horizontal, tr("Паспорт"));
        ui->tableView->model()->setHeaderData(15,   Qt::Horizontal, tr("Представители"));
        ui->tableView->model()->setHeaderData(16,   Qt::Horizontal, tr("Образование"));
        ui->tableView->model()->setHeaderData(17,   Qt::Horizontal, tr("Льготы"));
        /*ui->tableView->model()->setHeaderData(18,   Qt::Horizontal, tr("Достижения"));*/
        ui->tableView->model()->setHeaderData(18,   Qt::Horizontal, tr("Дополнительно"));
        break;
    }

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    if(table == STUDENT)
         header->hideSection(0);
    else header->showSection(0);

    refresh_menu();
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid() || currenttable == STUDENT)
        return ;

    int id = 0;
    Tables nexttable;

    if(!query->seek(index.row(),false))                                         /// позиционирование на необходимой записи
        return ;

    switch(currenttable){
    case SPECIALITY:
        nexttable = GROUP;
        id = query->value("idspeciality").toInt();
        break;
    case GROUP:
        nexttable = STUDENT;
        id = query->value("idgroup").toInt();
        break;
    }

    set_current_table(nexttable, MOUSEMODE, id);                                /// переключение таблиц
}

void MainWindow::on_Switch_table_spec_button_clicked()
{
    set_current_table(SPECIALITY, BUTTONMODE, 0);
}

void MainWindow::on_Switch_table_group_button_clicked()
{
    set_current_table(GROUP, BUTTONMODE, 0);
}

void MainWindow::on_Switch_table_stud_button_clicked()
{
    set_current_table(STUDENT, BUTTONMODE, 0);
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    if(currenttable != STUDENT) return ;                                        /// только для таблицы студенты
    if(index.column() < 12)     return ;                                        /// только для 12-19 столбцов таблицы
    if(!query->seek(index.row())) return ;                                      /// спозиционироваться на нужную строку

    int idsubject = query->value("subject").toInt();

    QString str1 = query->value("surname").toString();
    QString str2 = index.model()->headerData(index.column(),
                                             Qt::Horizontal).toString();
    subtabledlg->setTitleText(str1+" >> "+str2);
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
    if(cursor().pos().y() > (QApplication::desktop()->availableGeometry(0).height()/2 + 200))
        pos.setY(pos.y() - subtabledlg->height());
    ///тоже самое по X
    if(cursor().pos().x() > (QApplication::desktop()->availableGeometry(0).width()/2 + 500))
        pos.setX(pos.x() - subtabledlg->width());

    subtabledlg->move(pos);
    /// вывод диалогового окна с суб таблицей в качестве контекстного меню
    subtabledlg->ExecSqlQuery(subtable,idsubject);

}

void MainWindow::tableView_items_selected()
{
    int countselectedrows = ui->tableView->selectionModel()->selectedRows().size();
    bool satate = countselectedrows == 1;
    Table_record_edit->setEnabled(satate);
    ui->Edit_button1->setEnabled(satate);
    Status_label_count_selected->setText("Выделено строк: "+QString::number(countselectedrows));
}

void MainWindow::add_new_record()                                               /// добавить запись
{

}

void MainWindow::remove_records()                                               /// удалить записи
{

}

void MainWindow::edit_records()                                                 /// изменить записи
{
    QModelIndexList rowslist = ui->tableView->selectionModel()->selectedRows(0);/// получить список всех строк в которых выделен 0-й столбец
    if(rowslist.size() != 1) return ;                                           /// список содержит не одну строку -> завершить
    int row = rowslist[0].row();                                                /// получить номер выделенной строки
    query->first();
    for (;row != 0; row--){                                                     /// спозиционироваться на нужной записи
        query->next();
    }
    EditRecordModel* recordmodel = new EditRecordModel();                       /// модель данных из записи таблицы
    QList<QString> list;

    switch(currenttable){
    case SPECIALITY:
        /// подготовка модели данных записи (атрибут, значение)
        recordmodel->modelAddRow("Шифр",            query->value("idspeciality"));
        recordmodel->modelAddRow("Аббревиатура",    query->value("abbreviation"));
        recordmodel->modelAddRow("Наименование",    query->value("name"));
        recordmodel->modelAddRow("Период обучения", query->value("periodeducation"));
        recordmodel->modelAddRow("Базируется на",   query->value("basedon"));
        recordmodel->modelAddRow("Специализация",   query->value("specialization"));
        /// полготовка списка регулярных выражений для валидатора
        list.append("\\d{4}");
        list.append("\\d{4}");
        list.append("\\d{4}");
        list.append("\\d{4}");
        list.append("\\d{4}");
        list.append("\\d{4}");

        break;
    }
    dlgrecordedit.setModel(recordmodel,&list);


    dlgrecordedit.exec();
    query->first();
}

void MainWindow::on_Edit_button1_clicked()
{
    edit_records();
}
