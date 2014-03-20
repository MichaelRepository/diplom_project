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

    splashwindow->finish(&authorizedlg);
    /// создание окна для отображения подтаблиц
    subtablewidget = new SubTableWidget(connectionname, this);                  /// установка флага - "контекстное меню", для диалога субтаблиц
    subtablewidget->setWindowFlags(Qt::Popup);

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


    set_current_table(SPECIALITY, BUTTONMODE, 0);                               /// установить текущую таблицу
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
    /// переключить состояние кнопок и элементов контекстного меню, зависимых от числа выделенных записей
    ui->Edit_button1->setEnabled(state);
    Table_record_edit->setEnabled(state);
    /// обновить надписи в статус баре
    Status_label_count_rows->setText    (" Число записей: "+ QString::number(query->size())+" ");
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));
    Status_label_count_selected->setText("Выделено строк: "+ QString::number(countselectedrows));
}

void MainWindow::set_current_table(Tables table, ModeSwitchingTable mode,
                                   int keyvalue)
{
    query->finish();                                                            /// сбросить предыдущий результат запроса
    querymodel->clear();                                                        /// очистить модель данных запроса
    /// формирование запросов, относительно текущей таблицы
    QStringList headerList;                                                     /// список заголовков таблицы
    switch(table){
    case SPECIALITY:
        if(mode == BUTTONMODE) query->prepare("SELECT * "
                                              "FROM speciality "
                                              "ORDER BY idspeciality");
        else return;
        headerList << "Шифр"
                   << "Аббревиатура"
                   << "Наименование"
                   << "Период обучения"
                   << "Базируется на"
                   << "Специализация";
        break;
    case GROUP:
        if(mode == BUTTONMODE)
            query->prepare("SELECT * "
                           "FROM groups "
                           "ORDER BY idgroup");
        else{
            query->prepare("SELECT * "
                           "FROM groups "
                           "WHERE speciality = :id "
                           "ORDER BY idgroup");
            query->bindValue(":id",keyvalue);
        }
        headerList << "Номер"
                   << "Специальность"
                   << "Наименование"
                   << "Год формирования"
                   << "Форма обучения"
                   << "Бюджет"
                   << "Курс";
        break;
    case STUDENT:
        if(mode == BUTTONMODE)
            query->prepare("SELECT "
                           "student.subject, "
                           "student.numbertestbook, "
                           "subject.surname, "
                           "subject.name, "
                           "subject.patronymic, "
                           "subject.sex, "
                           "groups.name, "
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
                           "LEFT JOIN subject ON (student.subject = subject.idsubject) ");
        else{
            query->prepare("SELECT "
                           "student.subject, "
                           "student.numbertestbook, "
                           "subject.surname, "
                           "subject.name, "
                           "subject.patronymic, "
                           "subject.sex, "
                           "groups.name, "
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
                           "WHERE student.group = :id");
            query->bindValue(":id",keyvalue);
        }
        headerList << "#"
                   << "Номер зачетки"
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

    for(int i = 0; i < headerList.size(); ++i)
        ui->tableView->model()->setHeaderData(i,Qt::Horizontal,headerList.at(i));

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
    if(currenttable != STUDENT)   return ;                                      /// только для таблицы студенты
    if(index.column() < 12)       return ;                                      /// только для 12-19 столбцов таблицы
    if(!query->seek(index.row())) return ;                                      /// спозиционироваться на нужную строку

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
    query->first();
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
        reglist.append("\\d{1,6}");
        reglist.append("[\\w\\s]{1,50}");
        reglist.append("[\\w\\s]{1,50}");
        reglist.append("\\d{1,3}");
        reglist.append("[\\w\\s]{1,100}");
        reglist.append("[\\w\\s]{1,100}");
        break;
    case GROUP:{
        /// подготовка модели данных записи (атрибут, значение)
        /// получение списка всех существующих специальностей  в виде подтаблицы
        SubTableWidget* subtableviewer = new SubTableWidget(connectionname);    /// создается виджет для отображения субтаблицы
        QStringList headerlist;                                                 /// список заголовков таблицы
        QStringList defaultattribute;                                           /// список атрибутов, выводимых в редакторе строки
        headerlist << "Шифр"
                   << "Аббревиатура";
        defaultattribute << "idspeciality"
                         << "abbreviation";
        subtableviewer->setHeadersNameList(headerlist);                         /// установлен список заголовков
        subtableviewer->setDefaultAttributesList(defaultattribute);             /// установлен список атрибутов - по умолчанию
        /// передача текста запроса
        subtableviewer->ExecSqlQuery("SELECT "
                                     "idspeciality, "
                                     "abbreviation "
                                     "FROM speciality "
                                     "ORDER BY idspeciality");
        QList<QPair<QString,QVariant> > attrlist;                               /// список атр+знач для установки текущей строки в таблице
        attrlist.append(qMakePair(QString("idspeciality"),
                                  query->value("speciality") ));                /// строка быдет выбрана по значению атрибута speciality
        if(!subtableviewer->setCurrentRow(attrlist)){
            /// ОШИБКА, ЗАПИСЬ НЕ НАЙДЕНА
            return ;
        }

        tableattributelist.insert("Специальность", subtableviewer);             /// атрибут "Специальность" добавлена в список атриб, отображающихся в виде таблицы

        recordmodel.modelAddRow("Специальность",   query->value("speciality"));
        recordmodel.modelAddRow("Наименование",    query->value("name"));
        recordmodel.modelAddRow("Год создания",    query->value("yearformation"));
        recordmodel.modelAddRow("Форма обучения",  query->value("form"));
        recordmodel.modelAddRow("Бюджет",          query->value("budget"));
        recordmodel.modelAddRow("Курс",            query->value("course"));
        /// подготовка списка регулярных выражений для валидатора
        reglist.append("\\d{0,6}");
        reglist.append("[\\w\\s]{0,50}");
        reglist.append("[\\w\\s]{0,50}");
        reglist.append("\\d{0,3}");
        reglist.append("[\\w\\s]{0,100}");
        reglist.append("[\\w\\s]{0,100}");
        break;
    }
    }
    dlgrecordedit.setModel(&recordmodel, &reglist, &tableattributelist);

    dlgrecordedit.exec();
    dlgrecordedit.setFocus();


    //удаление временных данных
    QMap<QString, SubTableWidget*>::const_iterator i = tableattributelist.constBegin();
    while (i != tableattributelist.constEnd()) {
        delete i.value();
        ++i;
    }
    tableattributelist.clear();
    query->first();
}

void MainWindow::on_Edit_button1_clicked()
{
    edit_records();
}
