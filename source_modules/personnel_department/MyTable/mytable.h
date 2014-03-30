#ifndef MYTABLE_H
#define MYTABLE_H

#include <QTableView>
#include <QObject>
#include <QtSql>

#include "dbmessdlg.h"
#include "myeditrecordmodel.h"
#include "mysqlrecord.h"
#include "myinfoscheme.h"

enum Tables{SPECIALITY, GROUP, STUDENT};                                        /// список основных таблиц
enum extraTables{CITIZENSHIPLIST, PRIVILEGESCATEGORY, SCHOOL, TYPESCHOOL,       /// справочники - таблицы категорий
                 TYPEEDUCATION};

class MyTable{
public:
    QString     tablename;                                                      /// реальное имя таблицы нужно для FROM запроса

    MyTable(MyInfoScheme* metadatascheme, QString connectname);

    void appendField(QString field,         QString table = "",
                     bool editable = true,  bool visible = true);

    bool initializeTable();

    void setFilterForField(QString field, QString filter);                      /// фильтр для конкретного поля
    void setFilterForTable(QString str);                                        /// фильтр не привязанный к полю
    void setFilterActivity(bool activ);                                         /// установить активность фильтра
    void setCurrentRow (int index);
    bool updateData();                                                          /// запросить данные

    bool setUpdateDataFields();
    bool setInsertDataFields();

    bool     isFiltered();                                                      /// проверка активности фильтра
    int      getRecordsCount();                                                 /// получить число записей в таблице
    int      getFieldsCount();                                                  /// количество полей
    QString  getFieldAlterName  (int col);                                      /// получить альтернативное имя поле
    QString  getFieldName       (int col);                                      /// получить имя поля
    QVariant getCellValue       (int row, int col);                             /// получить значение ячейки таблицы
    QVariant getFieldValue      (int row, QString field);
    int      searchRecordByField(QString field, QVariant value);                /// получить номер строки по параметру поля
    int      getCurrentRowIndex();
    MySqlRecord *getRecord(int row);                                            /// получить запись (row присваевается как текущая строка)

    QSqlError lastSqlError();

private:
    bool preparePrimaryKeysData(); /// получение первичных ключей для каждой субтаблицы
    bool generateSelectQuery();
    void generateUpdateQuery();/// генерируется списки полей, участвующие в запросе update и insert относительно каждой таблицы
    bool prepareForeignKeysDataForTable(QString table);///  для каждой субтаблицы
    bool prepareForeignKeysData();


    QSqlError sqlerror;

    QString       connectionname;                                               /// имя подключения к БД
    MyInfoScheme* metadata;                                                     /// источник метаданных
/// данные для выполнения SELECT запроса
    QString       lastquerytext;                                                /// хранит текст последнего SELECT запроса для обновления данных таблицы
/// общие данные для отображения таблицы в программе
    int           currentrow;                                                   /// текущая строка таблицы
    QString       freeFilter;
    bool          filtered;
    QStringList   subtable;                                                     /// список таблиц, входящих в состав данного объекта (без повторений)
    QStringList   primarykeys;                                                  /// список первичных полей каждой таблицы (1ключ - 1таблица)!!!!
    QMap<int,int> indexesofvisiblecolumns;                                      /// индексы отображаемых колонок (index, realnum)
    QSqlQuery     tablequery;                                                   /// данные запроса для таблицы
    QMap<QString, QStringList> fieldsforquery;                                  /// список полей для формирвоания запроса на обновление таблицы

/// параметры полей
    QStringList fieldlist;                                                      /// имя поля
    QStringList tablenamelist;                                                  /// таблица поля (список таблиц, повторяются, т.к указаны для каждого поля)
    QStringList alternamelist;                                                  /// альтернативное название поля
    QStringList validatorlist;                                                  /// рег. выр. для валидатора
    QStringList alterfields;                                                    /// альтернативные поля, выводимые вместо текущего
    QStringList   filters;                                                      /// фильтр поля
    QVector<bool> flagseditable;                                                /// флаги видимости в редакторе
    QVector<bool> flagsvisible;                                                 /// флаги видимости
    QMap<int,MyDataReference> references;                                       /// поле - ссылка на внешнюю таблицу (index, данные ссылки)
/// данные для редактора записи таблицы
    MySqlRecord     editablerecord;                                             /// рабочая копия редактируемой записи

};
#endif // MYTABLE_H
