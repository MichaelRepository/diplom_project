#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <QMainWindow>
#include <QSplashScreen>
#include <QMouseEvent>
#include <QMessageBox>
#include <QtSql>
#include <QtSvg>
#include <QMenu>

/// хедеры для таблицы
#include "QSpreadsheetHeaderView.h"
/// виджет для субтаблиц
#include "subtablewidget.h"

#include "editrecordmodel.h"
#include "dialogeditrecord.h"
//#include "wowsqlmodel.h"
#include "messdlg.h"
#include "dbmessdlg.h"
#include "authorizdlg.h"

#include "mytable.h"

enum ModeSwitchingTable{BUTTONMODE, MOUSEMODE};                                 /// режим переключения между таблицами

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void    init_sys();                                                         /// подготовить систему
    QString get_connect_name();                                                 /// вернуть имя подключения к БД

private slots:
    void refresh_menu();                                                        /// обновить меню
    void set_current_table(Tables table, ModeSwitchingTable mode);                                       /// установить таблицу

    /// слоты - обработчики событий виджетов
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_Switch_table_spec_button_clicked();
    void on_Switch_table_group_button_clicked();
    void on_Switch_table_stud_button_clicked();
    void on_tableView_clicked(const QModelIndex &index);
    void on_Edit_button1_clicked();
    void tableView_items_selected();                                            /// обработка выделения элементов таблицы

private:
 /// методы
    void initTables();

    void add_new_record();                                                      /// добавить новую запись
    void remove_records();                                                      /// удалить записи
    void edit_records();                                                        /// изменить запись

 /// параметры
    Ui::MainWindow *ui;

    QSplashScreen*          splashwindow;                                       /// загрузочный экран
    /// дополнительные окна
    DialogEditRecord        dlgrecordedit;                                      /// редактор записи
    AuthorizDlg             authorizedlg;                                       /// авторизационное окно
    MessDlg                 messdlg;                                            /// диалог для вывода сообщения системы
    dbMessDlg               dbmessdlg;                                          /// диалог для вывода сообщеня БД
    SubTableWidget*         subtablewidget;                                     /// окно подтаблицы
    /// элементы статусбара
    QLabel*                 Status_label;                                       /// используется для создания отступа
    QLabel*                 Status_label_curtable;                              /// отображает название активной таблицы
    QLabel*                 Status_label_count_rows;                            /// отображает число строк в активной таблице
    QLabel*                 Status_label_count_selected;                        /// отображает число выделенных строк
    QLineEdit*              Status_search_edit;                                 /// поле быстрого поиска
    /// Элементы контекстного меню таблицы
    QAction*                Table_record_edit;
    QAction*                Table_record_add;
    QAction*                Table_record_remove;
    /// объекты для работы с СУБД
    QSqlQuery*              query;                                              /// текущий запрос к СУБД
    QSqlQueryModel*         querymodel;                                         /// модель данных запроса
    QSpreadsheetHeaderView* header;                                             /// заголовок таблицы (вьювера)
    /// служебные параметры и объекты
    int                     userid;                                             /// идентификатор пользователя
    QString                 connectionname;                                     /// имя для получения подключения к СУБД
    Tables                  currenttable;                                       /// активная таблица
    QString                 currentfilter;                                      /// активный фильтр
    bool                    filterchecked;                                      /// флаг фильтра (активен/неактивен)
    // ОБНОВЛЕНИЕ
    MyTable *globaltable;
    MyTable specialitytable;
    MyTable grouptable;
    MyTable studenttable;

    QSettings*              setting;                                            /// запись/чтение параметров приложения

};

#endif // MAINWINDOW_H
