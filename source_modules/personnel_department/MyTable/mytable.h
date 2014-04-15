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
class MyTable;

class MyLink{
public:
    MyLink(){
        table_1 = 0;
    }

    /// ссылка на объект MyTable из внешней таблицы (не субтаблицы!)
    QString name;                                                               /// имя ссылки
    QString key0_1;                                                             /// ключ связывающий t0 и t1 (родитель t0)
    QString key1_0;                                                             /// ключ связывающий t1 и t0 (родитель t1)
    MyTable *table_1;                                                           /// ссылающаяся таблица

    QString selectquerytext;                                                    /// текст запроса данных из таблицы t1
};

class MyField{
public:
    MyField () {
        table           = 0;
        realindex       = -1;
        indexforviewer  = -1;
        isEditable      = false;
        isVisible       = false;
        isForeign       = false;
        isPrimary       = false;
    }

    MySubtable* table;                                                          /// таблица к которой относится поле
    int realindex;                                                              /// реальный номер поля
    int indexforviewer;                                                         /// номер для вьювера
    QString name;                                                               /// имя
    QString altername;                                                          /// псевдоним
    QString validator;                                                          /// валидатор
    QString alterfield;                                                         /// альтернативное поле (отображаемое вместо него)
    QString filter;                                                             /// фильтр
    bool    isEditable;                                                         /// признак редактируемости
    bool    isVisible;                                                          /// признак видимости в вьювере
    bool    isForeign;                                                          /// признак поля-внешнего ключа
    bool    isPrimary;                                                          /// признак поля-первичного ключа
    MyDataReference reference;                                                  /// параметр - ссылка на внешнюю таблицу
    MyDataOfKey     dataofkey;                                                  /// данные внешнего ключа (если поле-внешний ключ)
};

class MySubtable{
public:
    QString                 name;                                               /// имя
    QList<MyField*>         fields;                                             /// указатели на поля (сами поля хранятся списком в объекте - таблица)
    QString                 primarykey;                                         /// ключевое поле
    /// внешние ключи, связывающие текущую субтаблицу с другими
    /// субтаблицами, входящими в состав структуры объекта MyTable
    QList<MyDataOfKey>      foreignkeys;
    int editablefieldscount;                                                    /// число редактируемых полей
};

class MyTable{
public:
    MyTable(MyInfoScheme* metadatascheme, QString connectname);
    void appendField(QString field, QString table, bool editable = true, bool visible = true);
    bool appendLink(QString name,   QString key0_1,
                     QString key1_0, MyTable *table_1);

    bool initializeTable();
    bool setUpdateDataFields();                                                 /// применить изменение данных в editablerecord
    bool setInsertDataFields();                                                 /// создать новую строку по данным из editablerecord
    bool removeRecord(int row);                                                 /// удалить строку из таблицы
    bool updateData();                                                          /// запросить данные

/// методы display... работают с полями из списка fieldforviewer (необходимы для привязки к модели данных)
    QString  displayFieldAlterName  (int index);                                /// получить альтернативное имя поля
    QString  displayFieldName       (int index);                                /// получить имя поля
    QVariant displayCellValue       (int row, int col);                         /// получить значение ячейки таблицы
    QVariant displayFieldValue      (int row, QString field);
    bool     displayedFieldIsForeign(int index);                                /// проверка на поле-внешний ключ
    bool     displayedFieldIsPrimary(int index);                                /// проверка на поле-превичный ключ
    bool     displayedFieldIsLink   (int index);                                /// проверка на поле-ссылку
    MyLink* const getdisplayedLink  (int index);
    int      displayedFieldsCount();                                            /// получить количество полей

    QVariant getFieldValue(QString field);                                      /// получить значение поля текущей записи

    int  getRecordsCount();                                                     /// получить число записей в таблице
    bool contains        (QString field);                                       /// проверка наличия поля по имени

    void setFilterForField(QString field, QString filter);                      /// фильтр для конкретного поля
    void setFilterForTable(QString str);                                        /// фильтр не привязанный к полю
    void setFilterActivity(bool activ);                                         /// установить активность фильтра
    void setCurrentRow (int index);

    bool     isFiltered();                                                      /// проверка активности фильтра
    QList<int> searchRecordByField(QString field, QString value);              /// получить номер строки по параметру поля
    int      getCurrentRowIndex();
    MySqlRecord *getRecord(int row);                                            /// получить запись (row присваевается как текущая строка)
    MySqlRecord *getEmptyRecordForInsert();                                     /// получить пустую строку, которая будет использована для заполнения и вставки в таблицу
    QSqlError lastSqlError();

private:
    int  fieldRealIndexOf(QString field);                                       /// получить реальный индекс поля по имени
    int  fieldViewUndexOf(QString field);                                       /// получить номер поля в вьювере по имени
    int  tableIndexOf    (QString table);                                       /// получить индекс субтаблицы

    bool checkSubtables();                                                      /// проверить наличие субтаблиц в бд
    bool prepareSubtables();                                                    /// получить PK и FK поля для субтаблиц
    bool getExtendedDataOfFields();                                             /// получить дополнительные данные для полей
    bool prepareLinks();
    void generateSelectQuery();

    QSqlError sqlerror;
    QSqlQuery tablequery;                                                       /// данные запроса для таблицы

    QString       connectionname;                                               /// имя подключения к БД
    MyInfoScheme* metadata;                                                     /// источник метаданных
    QString       lastselecttext;                                               /// хранит текст последнего SELECT запроса для обновления данных таблицы
    int           currentrow;                                                   /// текущая строка таблицы
    QString       freeFilter;
    bool          filtered;

    QList<MyField>    fields;                                                   /// поля таблицы
    QList<MyLink>     links;                                                    /// ссылки
    QList<MySubtable> subtables;                                                /// составные таблицы

    QList<MyField*>    fieldforviewer;                                          /// указатели на поля для вьювера
    QList<MyField*>    fieldforeditor;                                          /// указатели на поля для редактора
    MySqlRecord        editablerecord;                                          /// рабочая копия редактируемой записи

    bool init;                                                                  /// признак инициализированной таблицы
};
#endif // MYTABLE_H
