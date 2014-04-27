#ifndef MYTABLE_H
#define MYTABLE_H

#include <QtSql>

#include "mysqlrecord.h"
#include "myinfoscheme.h"
#include "myfilterdata.h"

class MyTable;

class MyLink{                                                                   /// ссылка на объект MyTable из внешней таблицы (не субтаблицы!)
public:
    MyLink(){ table_1 = 0; }
    QString name;                                                               /// имя ссылки
    QString key0_1;                                                             /// ключ связывающий t0 и t1 (родитель t0)
    QString key1_0;                                                             /// ключ связывающий t1 и t0 (родитель t1)
    MyTable *table_1;                                                           /// ссылающаяся таблица
    QString selectquerytext;                                                    /// текст запроса данных из таблицы t1
};

class MyTable{
public:
    MyTable(MyInfoScheme* metadatascheme, const QString& connectname);
    ~MyTable();

    void appendField(const QString& field, const QString& table,
                     bool editable = true, bool visible = true);
    bool appendLink(const QString& name,   const QString& key0_1,
                    const QString& key1_0, MyTable *table_1);

    bool initializeTable();
    bool setUpdateDataFields();                                                 /// применить изменение данных в editablerecord
    bool setInsertDataFields();                                                 /// создать новую строку по данным из editablerecord

    bool removeRecord(int row);                                                 /// удалить строку из таблицы
    bool removeRecords(const QList<int> records);
    bool updateData();                                                          /// запросить данные

    bool setUpdateGroupOfRecords(const QList<int> &records);                    /// внести изменения нескольких полей в группу записей (данные беруться из editablerecord)
    bool setUpdateGroupOfRecords(const QList<int> &records,                     /// внести изменения нескольких полей в группу записей
                                 const QList<QPair<QString, QString> > &field_value_list);

/// методы display... работают с полями из списка fieldforviewer (необходимы для привязки к модели данных)
    QString  displayFieldAlterName  (int index) const;                          /// получить альтернативное имя поля
    QString  displayFieldName       (int index) const;                          /// получить имя поля
    QVariant displayCellValue       (int row, int col);                         /// получить значение ячейки таблицы
    QVariant displayFieldValue      (int row, const QString& field);
    QString  displayFieldRealType   (int index);
    bool     displayedFieldIsForeign(int index) const;                          /// проверка на поле-внешний ключ
    bool     displayedFieldIsPrimary(int index) const;                          /// проверка на поле-превичный ключ
    bool     displayedFieldIsLink   (int index) const;                          /// проверка на поле-ссылку
    MyLink*  getdisplayedLink       (int index);
    int      displayedFieldsCount() const;                                      /// получить количество полей
    int      getRecordsCount() const;                                           /// получить число записей в таблице

    QVariant getFieldValue(const QString& field);                               /// получить значение поля текущей записи
    bool contains         (const QString& field) const;                         /// проверка наличия поля по имени

    void createFilterForRecords (const QList<int> &records);
    void createQuickFilter(const QString& field, const QString& f_operator,     /// быстрый фильтр для переключения между таблицами по двойному щелчку
                           const QString& value);
    void setFilterActivity(bool activ);                                         /// установить активность фильтра

    void setCurrentRow (int index);                                             /// установить текущую строку
    int  getCurrentRowIndex() const;                                            /// получить номер текущей строки(записи)
    bool        isFiltered() const;                                             /// проверка активности фильтра
    QList<int>  searchRecordByField(QString field, QString value);              /// получить номер строки по параметру поля
    MySqlRecord *getRecord(int row);                                            /// получить запись (row присваевается как текущая строка)
    MySqlRecord *getEmptyRecordForInsert();                                     /// получить пустую строку, которая будет использована для заполнения и вставки в таблицу
    QSqlError   lastSqlError() const;

    MyFilterNode* getFilterRoot()const;

    QList<const MyField *> getFields() const;                                   /// получить список отображаемых полей текущего объекта MyTable
    QList<const MyField *> getDisplayedFields() const;
    QList<const MyField *> getDisplayedLinkedTableFields() const;               /// получить список полей всех слинкованных таблиц
    QList<const MyField *> getJoinedFieldsList() const;                         /// получить объединенный список полей - текущий+слинкованный

    QMap<int, QString> getCustomOrdersData() const;                             /// получить список отсортированных полей
    void setCustomOrderData(int column, QString data);
    void clearCustomOrdersData();

    QSqlQuery  getTableQueryData() const;
    QList<int> fieldsIndexOfColumns(const QList<int> &columns) const;

private:
    int  fieldRealIndexOf(QString field) const;                                 /// получить реальный индекс поля по имени
    int  fieldRealIndexOf(int col) const;
    int  fieldViewIndexOf(QString field) const;                                 /// получить номер поля в вьювере по имени
    int  tableIndexOf    (QString table) const;                                 /// получить индекс субтаблицы

    bool checkSubtables();                                                      /// проверить наличие субтаблиц в бд
    bool prepareSubtables();                                                    /// получить PK и FK поля для субтаблиц
    bool getExtendedDataOfFields();                                             /// получить дополнительные данные для полей
    bool prepareLinks();
    void generateSelectQuery();

    QSqlError sqlerror;
    QSqlQuery tablequery;                                                       /// данные запроса для таблицы

    QString       connectionname;                                               /// имя подключения к БД
    MyInfoScheme* metadata;                                                     /// источник метаданных
    int           currentrow;                                                   /// текущая строка таблицы

    QList<MyField>    fields;                                                   /// поля таблицы
    QList<MyLink>     links;                                                    /// ссылки
    QList<MySubtable> subtables;                                                /// составные таблицы

    QList<MyField*>    fieldforviewer;                                          /// указатели на поля для вьювера
    QList<MyField*>    fieldforeditor;                                          /// указатели на поля для редактора
    MySqlRecord        editablerecord;                                          /// рабочая копия редактируемой записи

    MySelectStruct mainselect;                                                  /// основная структура селект запроса для таблицы, без фильтрации
    MyFilterNode*  filter;                                                      /// фильтр
    QString        mainselecttext;                                              /// основной SELECT запроса, сформированный при инициализации таблицы
    bool init;                                                                  /// признак инициализированной таблицы
    bool filtered;
};
#endif // MYTABLE_H
