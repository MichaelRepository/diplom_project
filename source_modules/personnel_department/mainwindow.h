#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QDateTime>

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

#include "dialogeditrecord.h"
#include "messdlg.h"
#include "dbmessdlg.h"
#include "authorizdlg.h"
#include "myfilterform.h"

#include "MyTable/myinfoscheme.h"
#include "MyTable/mytable.h"

#include "MyTable/myeditrecordmodel.h"
#include "MyTable/mytablemodel.h"

#include "QuaZIP/quazip.h"
#include "QuaZIP/quazipfile.h"
#include "MyReport/mydocumentodf.h"

enum Tables{SPECIALITY, GROUP, STUDENT};                                        /// список основных таблиц

/*enum extraTables{CITIZENSHIPLIST,
 * PRIVILEGESCATEGORY,
 * SCHOOL,
 * TYPESCHOOL,
 * TYPEEDUCATION};*/

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
    void set_current_table(Tables table);                                       /// установить таблицу

    /// слоты - обработчики событий виджетов
    void on_Switch_table_spec_button_clicked();
    void on_Switch_table_group_button_clicked();
    void on_Switch_table_stud_button_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_tableView_clicked      (const QModelIndex &index);

    void on_Refresh_button1_clicked();
    void on_Edit_button1_clicked();
    void on_Add_button1_clicked();
    void on_Delete_button1_clicked();
    void on_Filter_checked_button_clicked();
    void on_Master_filter_button_clicked();

    void searchStart();
    void searchBtMenuTriggered(QAction* action);

    void subtableRecordEdit();
    void subtableRecordAdd();
    void subtableRecordRemove();
    void subtableRowSelected(int row);
    void setGlobalTableFiltered();
    void tableView_items_selected();                                            /// обработка выделения элементов таблицы

    void on_Filter_selected_button_clicked();
    void on_Filter_unselected_button_clicked();

    void sortUP  (int column, QString name);
    void sortDown(int column, QString name);
    void clearOrders();

    void on_tabWidget_currentChanged(int index);
    void on_Refresh_button2_clicked();
    void on_Create_Report_bt1_clicked();

private:
    void initTables();
    void add_new_record (MyTable *table);                                       /// добавить новую запись
    void remove_records (MyTable *table);                                       /// удалить записи
    void edit_records   (MyTable *table);                                       /// изменить запись
    bool refresh_table  (MyTable *table);

    void getReportTemplatesTable();                                             /// запросить данне с шаблонов для текущего пользователя
    void templatSelected(const QModelIndex &current,
                         const QModelIndex &previous);
    void createReport();

    void createFilterForSelectedRecords();
    void createFilterForUnSelectedRecords();

    void prepareSearchData(const QList<const MyField *>& fields,
                           QMap<QString, QString>& data, QMenu *menu);

    Ui::MainWindow *ui;
    QSplashScreen*          splashwindow;                                       /// загрузочный экран
    /// дополнительные окна
    DialogEditRecord*       dlgrecordedit;                                      /// редактор записи
    AuthorizDlg             authorizedlg;                                       /// авторизационное окно
    dbMessDlg               dbmessdlg;                                          /// диалог для вывода сообщеня БД
    SubTableWidget*         subtablewidget;                                     /// окно подтаблицы
    MyFilterForm*           tablefilter;
    /// элементы статусбара
    QLabel*                 Status_label_curtable;                              /// отображает название активной таблицы
    QLabel*                 Status_label_count_rows;                            /// отображает число строк в активной таблице
    QLabel*                 Status_label_count_selected;                        /// отображает число выделенных строк
    QFrame*                 Status_search_frame;
    QLineEdit*              Status_search_edit;                                 /// поле быстрого поиска
    QToolButton*            Status_search_bt;
    /// Элементы контекстного меню таблицы
    QAction*                Table_refresh;
    QAction*                Table_record_edit;
    QAction*                Table_record_add;
    QAction*                Table_record_remove;
    QAction*                Table_set_filtered;
    QAction*                Table_master_filter;

    QSpreadsheetHeaderView* header;                                             /// заголовок таблицы (вьювера)

    /// служебные параметры и объекты
    int                     userid;                                             /// идентификатор пользователя
    QString                 connectionname;                                     /// имя для получения подключения к СУБД
    Tables                  currenttable;                                       /// активная таблица
    /// источник метаданных
    MyInfoScheme* metadatasource;
    /// модель данных для отобраения таблицы
    MyTableModel*   tablemodel;
    MyTableModel*   subtablemodel;
    QSqlQueryModel* reportstablemodel;
    /// Таблицы
    MyTable *globaltable;                                                       /// глобальный указатель на текущую главную таблицу
    MyTable *globalsubtable;                                                    /// глобальный указатель на текущую субтаблицу
    /// указатели на реальные объекты
    MyTable* specialitytable;
    MyTable* grouptable;
    MyTable* studenttable;
    MyTable* citizenshiptable;
    MyTable* residencetable;

    /// данные для поискового элемента
    QMap<QString, QString> search_data_for_spec;
    QMap<QString, QString> search_data_for_group;
    QMap<QString, QString> search_data_for_stud;
    QString current_search_key_for_spec;
    QString current_search_key_for_group;
    QString current_search_key_for_stud;
    QMenu* search_menu_for_spec;
    QMenu* search_menu_for_group;
    QMenu* search_menu_for_stud;

    QSettings* setting;                                                         /// запись/чтение параметров приложения
};

#endif // MAINWINDOW_H
