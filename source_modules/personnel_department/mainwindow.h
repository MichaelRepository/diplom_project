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
/// диалог для субтаблиц
#include "subtabledialog.h"

#include "editrecordmodel.h"
#include "dialogeditrecord.h"
#include "wowsqlmodel.h"
#include "messdlg.h"
#include "dbmessdlg.h"
#include "authorizdlg.h"


enum Tables{SPECIALITY, GROUP, STUDENT};                                        /// список основных таблиц
enum extraTables{CITIZENSHIPLIST, PRIVILEGESCATEGORY, SCHOOL, TYPESCHOOL,       /// справочники - таблицы категорий
                 TYPEEDUCATION};
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
    int     get_selected_rows();                                                /// вернуть число выделенных строк в таблице

private slots:
    void refresh_menu();                                                        /// обновить меню
    void set_current_table(Tables table, ModeSwitchingTable mode,
                           int keyvalue);                                       /// установить таблицу

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
    void add_new_record();                                                      /// добавить новую запись
    void remove_records();                                                      /// удалить записи
    void edit_records();                                                        /// изменить запись

 /// параметры
    Ui::MainWindow *ui;

    QSplashScreen*  splashwindow;                                               /// загрузочный экран
    QSettings*      setting;                                                    /// запись/чтение параметров приложения

    DialogEditRecord    dlgrecordedit;
    AuthorizDlg         authorizedlg;
    MessDlg             messdlg;
    dbMessDlg           dbmessdlg;

    SubTableDialog*     subtabledlg;

    QLabel*         Status_label;                                               /// используется для создания отступа
    QLabel*         Status_label_curtable;                                      /// отображает название активной таблицы
    QLabel*         Status_label_count_rows;                                    /// отображает число строк в активной таблице
    QLabel*         Status_label_count_selected;                                /// отображает число выделенных строк
    QLineEdit*      Status_search_edit;                                         /// поле быстрого поиска

                                                                                /// Элементы контекстного меню таблицы
    QAction*        Table_record_edit;
    QAction*        Table_record_add;
    QAction*        Table_record_remove;

    QSqlQuery*              query;                                              /// текущий запрос к СУБД
    QSqlQueryModel*         querymodel;                                         /// модель данных
    QSpreadsheetHeaderView* header;                                             /// заголовок таблицы (вьювера)

    int     userid;                                                             /// идентификатор пользователя
    QString connectionname;                                                     /// имя для получения подключения к СУБД
    Tables  currenttable;                                                       /// активная таблица
    QString currentfilter;                                                      /// активный фильтр
    bool    filterchecked;                                                      /// флаг фильтра (активен/неактивен)
};

#endif // MAINWINDOW_H
