#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectionname = "wow_such";
    /// создание загрузчика параметров приложения
    setting = new QSettings("../myapp.ini",QSettings::IniFormat,this);

    querymodel = new QSqlQueryModel(this);


    /// подготовка подсказок для компонентов
    ui->thumbLabel->setToolTip      ("Миниатюра документа.");
    ui->RepName->setToolTip         ("Название документа отчета.");
    ui->RepInfo->setToolTip         ("Описание отчета.");
    ui->reportsTableView->setToolTip("Таблица шаблонов отчетов.");
    ui->showRepBt->setToolTip       ("Создать отчет.\nПозволяет проверить правильность созданного шаблона.");
    ui->saveRepBt->setToolTip       ("Позволяет сделать локальную копию шаблона.");
    ui->editRepBt->setToolTip       ("Вызывает диалоговое окно для изменения имени, описания и шаблона отчета.");
    ui->addRepBt->setToolTip        ("Вызывает диалоговое окно для описания нового отчета.");
    ui->remRepBt->setToolTip        ("Удаляет выделенный в таблице отчет.");


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_sys()
{
    AuthorizDlg adlg;
    dbMessDlg   ddlg;

    /// подключение к базе данных
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QMYSQL",connectionname);                    /// проверка наличия драйвера QMYSQL
    if (!db.isValid() )
    {
        ddlg.showdbmess(db.lastError());
        exit(0);
    }

    /// авторизационные данные группы операторов АИС
    /// загрузка из файла
    setting->beginGroup("SqlDatabase");
    db.setHostName     (setting->value("HostName",      "").toString());
    db.setPort         (setting->value("Port",          "").toInt());
    db.setDatabaseName (setting->value("DatabaseName",  "").toString());
    setting->endGroup();


    int  res;
    bool connect = false;
    while(!connect && res != 0)
    {
        res = adlg.exec();

        if(res == 0) exit(0);

        db.setUserName(adlg.userName());
        db.setPassword(adlg.userPass());

        /// открытие соединеня
        connect = db.open();
        if(!connect) ddlg.showdbmess(db.lastError());
    }

    getUserGroups();
}

QMap<int, QString> MainWindow::getGroupsNameList()
{
    if(usergroups.size() == 0) return QMap<int, QString> ();

    QList<MyUserGroup>::const_iterator itr;
    QMap<int, QString>  grouplist;
    for(itr = usergroups.begin(); itr != usergroups.end(); ++itr)
    {
        grouplist.insert((*itr).id, (*itr).name );
    }
    return grouplist;
}

void MainWindow::showReportsTable()
{
    ui->RepInfo->clear();
    ui->RepName->clear();
    ui->thumbLabel->clear();

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.isOpen())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
    }
    QString querytext = "SELECT reports.idreport, reports.repname, reports.repinfo, "
                        "reports.usergroup, usersgroup.namegroup "
                        "FROM reports, usersgroup "
                        "WHERE usersgroup.idgroup = reports.usergroup";
    querymodel->setQuery(querytext,db);
    if(querymodel->lastError().isValid())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(querymodel->lastError());
    }
    else
    {
        ui->reportsTableView->setModel(querymodel);
        ui->reportsTableView->horizontalHeader()->setFocusPolicy(Qt::NoFocus);

        ui->reportsTableView->model()->setHeaderData(0,Qt::Horizontal,QVariant("#"));
        ui->reportsTableView->model()->setHeaderData(1,Qt::Horizontal,QVariant("Название"));
        ui->reportsTableView->model()->setHeaderData(2,Qt::Horizontal,QVariant("Описание"));
        ui->reportsTableView->model()->setHeaderData(3,Qt::Horizontal,QVariant("# группы"));
        ui->reportsTableView->model()->setHeaderData(4,Qt::Horizontal,QVariant("Пользователи"));

        ui->reportsTableView->verticalHeader()->setMaximumSectionSize(25);

        ui->reportsTableView->resizeColumnsToContents();
        ui->reportsTableView->resizeRowsToContents();

        ui->reportsTableView->horizontalHeader()->resizeSection(2,100);

        QItemSelectionModel *sm = ui->reportsTableView->selectionModel();
        connect(sm,   &QItemSelectionModel::currentChanged,
                this, &MainWindow::repTableItemSelected);

        ui->reportsTableView->setFocus();

        sm->setCurrentIndex(querymodel->index(0,0), QItemSelectionModel::ClearAndSelect |
                   QItemSelectionModel::Rows);
    }
}

void MainWindow::addReport()
{
    ReportConfigDialog repconf;
    repconf.setGroupList(getGroupsNameList() );
    if(!repconf.exec() ){ this->activateWindow(); return;}
    this->activateWindow();

    /// подготовка данных для отправки в БД
    QString name  = repconf.repName();
    QString info  = repconf.repInfo();
    QString file  = repconf.repFile();
    int usergroup = repconf.userGroup();

    QByteArray thumbnail;
    QByteArray odtfiledata;
    getOdtFileData(file, odtfiledata, thumbnail);

    if(thumbnail.size() == 0 || odtfiledata.size() == 0)
    {
        QMessageBox mess;
        mess.setWindowIcon(this->windowIcon());
        mess.setWindowTitle("Ошибка");
        mess.setText("Невозможно получить данные файла отчета.\n"
                     "Отчет не будет добавлен в базу!");
        mess.addButton(QMessageBox::Ok);
        mess.setButtonText(QMessageBox::Ok, "Ясно");
        mess.setIcon(QMessageBox::Critical);
        mess.exec();
        return ;
    }

    /// подготовка запроса к БД
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    QSqlQuery query(db);
    db.transaction();
    query.prepare("INSERT INTO reports "
                  "(repname, repinfo, repico, repdata, usergroup) "
                  "VALUES (?,?,?,?,?) ");
    query.bindValue(0,QVariant(name) );
    query.bindValue(1,QVariant(info) );
    query.bindValue(2,QVariant(thumbnail) );
    query.bindValue(3,QVariant(odtfiledata) );
    query.bindValue(4,QVariant(usergroup) );
    if(!query.exec() )
    {
        dbMessDlg mess;
        mess.showdbmess(query.lastError() );
        db.rollback();
        return ;
    }
    db.commit();

    QMessageBox mess;
    mess.setWindowIcon(this->windowIcon());
    mess.setWindowTitle("Отчет добавлен");
    mess.setText("Данные отчета добавлены в БД");
    mess.addButton(QMessageBox::Ok);
    mess.setButtonText(QMessageBox::Ok, "Ясно");
    mess.setIcon(QMessageBox::Information);
    mess.exec();
    showReportsTable();
}

void MainWindow::remReport()
{
    QModelIndex index = ui->reportsTableView->currentIndex();
    if(!index.isValid()) return;
    int row = index.row();
    int id  = index.model()->index(row,0).data().toInt();
    QString repname = index.model()->index(row,1).data().toString();

    QMessageBox mess;

    mess.setWindowIcon(this->windowIcon());
    mess.setWindowTitle("Удаление записи");
    mess.setText("Вы собираетесь удалить отчет "+repname+"  из БД");
    mess.setIcon(QMessageBox::Warning);
    mess.addButton(QMessageBox::Abort);
    mess.addButton(QMessageBox::Apply);
    mess.setButtonText(QMessageBox::Abort, "Отменить");
    mess.setButtonText(QMessageBox::Apply, "Подтвердить");
    QAbstractButton *button1 = mess.button(QMessageBox::Abort);
    QAbstractButton *button2 = mess.button(QMessageBox::Apply);
    mess.exec();
    if(mess.clickedButton() == button1 || mess.clickedButton() == 0) return;

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    QSqlQuery query(db);
    db.transaction();
    query.prepare("DELETE FROM reports WHERE idreport = ?");
    query.bindValue(0, QVariant(id));
    if(!query.exec())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(query.lastError());
        db.rollback();
        return ;
    }
    db.commit();

    mess.removeButton(button1);
    mess.removeButton(button2);

    mess.setWindowTitle("Удаление записи");
    mess.setText("Отчет "+repname+" удален из БД");
    mess.setIcon(QMessageBox::Information);
    mess.addButton(QMessageBox::Ok);
    mess.setButtonText(QMessageBox::Ok, "Ясно");
    mess.exec();

    showReportsTable();
}

void MainWindow::editReport()
{
    QModelIndex index = ui->reportsTableView->currentIndex();
    if(!index.isValid()) return;
    int row = index.row();
    int id  = index.model()->index(row,0).data().toInt();
    QString repname = index.model()->index(row,1).data().toString();
    QString repinfo = index.model()->index(row,2).data().toString();

    ReportConfigDialog repconf;
    repconf.setGroupList(this->getGroupsNameList());
    repconf.setData(repname,repinfo,id,EDITMODE);
    if(!repconf.exec() ) return;

    /// получение обновленных данных
    repname         = repconf.repName();
    repinfo         = repconf.repInfo();
    int usergroup   = repconf.userGroup();
    QString file    = repconf.repFile();

    QByteArray thumbnail;
    QByteArray odtfiledata;

    if(file.size() != 0)
    {
        getOdtFileData(file, odtfiledata, thumbnail);

        if(thumbnail.size() == 0 || odtfiledata.size() == 0)
        {
            QMessageBox mess;
            mess.setWindowIcon(this->windowIcon());
            mess.setWindowTitle("Ошибка");
            mess.setText("Невозможно получить данные файла отчета.\n"
                         "Отчет не будет добавлен в базу!");
            mess.addButton(QMessageBox::Ok);
            mess.setButtonText(QMessageBox::Ok, "Ясно");
            mess.setIcon(QMessageBox::Critical);
            mess.exec();
            return ;
        }
    }

    /// подготовка запроса к БД
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    QSqlQuery query(db);
    db.transaction();
    if(file.size() > 0)
    {
        query.prepare("UPDATE reports SET "
                      "repname = ?, repinfo = ?, repico = ?, repdata = ?, usergroup = ? "
                      "WHERE idreport = ?");
        query.bindValue(0,QVariant(repname) );
        query.bindValue(1,QVariant(repinfo) );
        query.bindValue(2,QVariant(thumbnail) );
        query.bindValue(3,QVariant(odtfiledata) );
        query.bindValue(4,QVariant(usergroup) );
        query.bindValue(5,QVariant(id) );
    }
    else
    {
        query.prepare("UPDATE reports SET "
                      "repname = ?, repinfo = ?, usergroup = ? "
                      "WHERE idreport = ?");
        query.bindValue(0,QVariant(repname) );
        query.bindValue(1,QVariant(repinfo) );
        query.bindValue(2,QVariant(usergroup) );
        query.bindValue(3,QVariant(id) );
    }

    if(!query.exec() )
    {
        dbMessDlg mess;
        mess.showdbmess(query.lastError() );
        db.rollback();
        return ;
    }
    db.commit();

    QMessageBox mess;
    mess.setWindowIcon(this->windowIcon());
    mess.setWindowTitle("Отчет обновлен");
    mess.setText("Данные отчета успешно обновлены");
    mess.addButton(QMessageBox::Ok);
    mess.setButtonText(QMessageBox::Ok, "Ясно");
    mess.setIcon(QMessageBox::Information);
    mess.exec();

    showReportsTable();
}

void MainWindow::saveCopyRepTemp()
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

    QFileDialog filedlg;
    QString newfile = filedlg.getSaveFileName(0,"Сохранение файла","","ODT files (*.odt)");
    this->activateWindow();
    if(newfile.isEmpty() ) return;

    QFile file(newfile);
    file.open(QIODevice::WriteOnly);
    file.write(odtdata);
    file.close();

    QDesktopServices serv;

    if(!serv.openUrl(QUrl::fromLocalFile(newfile)) )
    {
        QMessageBox mess;
        mess.setWindowTitle("Ошибка");
        mess.setText("Невозможно открыть документ.");
        mess.setIcon(QMessageBox::Critical);
        mess.addButton(QMessageBox::Ok);
        mess.setButtonText(QMessageBox::Ok, "Ясно");
        mess.exec();
        return ;
    }
    this->activateWindow();
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

void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch(index){
    case 0: break;
    case 1: break;
    case 2:
        showReportsTable();
        break;
    }
}

void MainWindow::on_addRepBt_clicked()
{
    addReport();
}

void MainWindow::getUserGroups()
{
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.isOpen())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
    }
    QString querytext = "SELECT * FROM usersgroup";
    QSqlQuery query(db);
    if(!query.exec(querytext) )
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(query.lastError());
    }
    while(query.next())
    {
        MyUserGroup group;
        group.id    = query.value("idgroup").toInt();
        group.name  = query.value("namegroup").toString();
        group.info  = query.value("info").toString();
        usergroups.append(group);
    }
}

void MainWindow::repTableItemSelected(const QModelIndex & cur,
                                      const QModelIndex & )
{
    int row = cur.row();
    int id  = cur.model()->index(row,0).data().toInt();
    QString name = cur.model()->index(row,1).data().toString();
    QString inf  = cur.model()->index(row,2).data().toString();
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

void MainWindow::getOdtFileData(const QString &file, QByteArray &odtdata, QByteArray &thumbdata)
{
    /// подготовка файла
    QuaZip zip(file);
    zip.open(QuaZip::mdUnzip);

    for(bool f = zip.goToFirstFile(); f; f = zip.goToNextFile() )
    {
        if(zip.getCurrentFileName() == "Thumbnails/thumbnail.png")
        {
            QuaZipFile zipfile(&zip);
            zipfile.open(QIODevice::ReadOnly);
            thumbdata = zipfile.readAll();
            zipfile.close();
            break;
        }
    }
    zip.close();

    QFile odtfile(file);
    odtfile.open(QIODevice::ReadOnly);
    odtdata = odtfile.readAll();
    odtfile.close();
}

void MainWindow::on_remRepBt_clicked()
{
    remReport();
}

void MainWindow::on_editRepBt_clicked()
{
    editReport();
}

void MainWindow::on_showRepBt_clicked()
{
    createReport();
}

void MainWindow::on_saveRepBt_clicked()
{
    saveCopyRepTemp();
}
