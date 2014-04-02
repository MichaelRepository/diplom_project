#ifndef MYTABLE_H
#define MYTABLE_H

#include <QTableView>
#include <QObject>
#include <QtSql>
#include <QtAlgorithms>

#include "dbmessdlg.h"
#include "myeditrecordmodel.h"
#include "mysqlrecord.h"
#include "myinfoscheme.h"

enum Tables{SPECIALITY, GROUP, STUDENT};                                        /// список основных таблиц
enum extraTables{CITIZENSHIPLIST, PRIVILEGESCATEGORY, SCHOOL, TYPESCHOOL,       /// справочники - таблицы категорий
                 TYPEEDUCATION};

class MySubtable;

class MyField{
public:
    MySubtable* table;                                                          /// таблица к которой относится поле
    int realindex;                                                              /// реальный номер поля
    int indexforviewer;                                                         /// номер  для вьювера
    QString name;                                                               /// имя
    QString altername;                                                          /// псевдоним
    QString validator;                                                          /// валидатор
    QString alterfield;                                                         /// альтернативное поле (отображаемое вместо него)
    QString filter;                                                             /// фильтр
    bool    isEditable;                                                         /// признак редактируемости
    bool    isVisible;                                                          /// признак видимости в вьювере
    MyDataReference reference;                                                  /// параметр - ссылка на внешнюю таблицу
};

class MySubtable{
public:
    QString                 name;                                               /// имя
    QList<MyField*>         fields;                                             /// указатели на поля (сами поля хранятся списком в объекте - таблица)
    QString                 primarykey;                                         /// ключевое поле
    QList<MyDataOfKey>      foreignkeys;                                        /// внешние ключи
    int editablefieldscount;                                                    /// число редактируемых полей
};

class MyTable{
public:
    MyTable(MyInfoScheme* metadatascheme, QString connectname);
    void appendField(QString field, QString table, bool editable = true, bool visible = true);

    bool initializeTable();

    QString  displayFieldAlterName  (int index);                                /// получить альтернативное имя поля
    QString  displayFieldName       (int index);                                /// получить имя поля
    QVariant displayCellValue       (int row, int col);                         /// получить значение ячейки таблицы
    QVariant displayFieldValue      (int row, QString field);
    int      getRecordsCount();                                                 /// получить число записей в таблице
    int      getDisplayedFieldsCount();                                         /// количество полей

    bool setUpdateDataFields();
    bool setInsertDataFields();

    void setFilterForField(QString field, QString filter);                      /// фильтр для конкретного поля
    void setFilterForTable(QString str);                                        /// фильтр не привязанный к полю
    void setFilterActivity(bool activ);                                         /// установить активность фильтра
    void setCurrentRow (int index);
    bool updateData();                                                          /// запросить данные

    bool removeRecord(int row);                                                 /// удалить строку из таблицы

    bool     isFiltered();                                                      /// проверка активности фильтра
    int      searchRecordByField(QString field, QVariant value);                /// получить номер строки по параметру поля
    int      getCurrentRowIndex();
    MySqlRecord *getRecord(int row);                                            /// получить запись (row присваевается как текущая строка)
    MySqlRecord *getEmptyRecordForInsert();                                     /// получить пустую строку, которая будет использована для заполнения и вставки в таблицу
    QSqlError lastSqlError();

private:
    bool contains        (QString field);
    int  fieldRealIndexOf(QString field);
    int  fieldViewUndexOf(QString field);
    int  tableIndexOf    (QString table);                                       /// получить индекс субтаблицы

    bool checkSubtables();                                                      /// проверить наличие субтаблиц в бд
    bool prepareSubtables();                                                    /// получить PK и FK поля для субтаблиц
    bool getExtendedDataOfFields();                                             /// получить дополнительные данные для полей

    void generateSelectQuery();

    QSqlError sqlerror;
    QSqlQuery tablequery;                                                       /// данные запроса для таблицы

    QString       connectionname;                                               /// имя подключения к БД
    MyInfoScheme* metadata;                                                     /// источник метаданных
    QString       lastselecttext;                                               /// хранит текст последнего SELECT запроса для обновления данных таблицы
    int           currentrow;                                                   /// текущая строка таблицы
    QString       freeFilter;
    bool          filtered;

    QList<MyField>    fields;                                                   /// поля
    QList<MySubtable> subtables;                                                /// составные таблицы

    QList<MyField*>    fieldforviewer;                                          /// поля для вьювера
    QList<MyField*>    fieldforeditor;                                          /// поля для редактора

    MySqlRecord        editablerecord;                                          /// рабочая копия редактируемой записи

};
#endif // MYTABLE_H
