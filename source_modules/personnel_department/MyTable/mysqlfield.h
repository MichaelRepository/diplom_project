#ifndef MYSQLFIELD_H
#define MYSQLFIELD_H

#include <QObject>
#include <QtSql>
#include <QDebug>

class MyDataOfKey{
public:
    QString table;              /// таблица которая ссылается   (slave)
    QString referencedtable;    /// таблица на которую сслаются (master)
    QString foreignkey;         /// внешний ключ
    QString primarykey;         /// первичный ключ
};

class MyField;

class MySubtable{
public:
    QString                 name;                                               /// имя
    QString                 altername;
    QList<MyField*>         fields;                                             /// указатели на поля (сами поля хранятся списком в объекте - таблица)
    QString                 primarykey;                                         /// ключевое поле
    /// внешние ключи, связывающие текущую субтаблицу с другими
    /// субтаблицами, входящими в состав структуры объекта MyTable
    QList<MyDataOfKey>      foreignkeys;
    int editablefieldscount;                                                    /// число редактируемых полей
};

class MyDataReference{                                                          /// класс для хранения данных поля - ссылки
public:
    QString     sqltext;                                                        /// запрос на внешнюю таблицу
    QStringList alternames;                                                     /// алтернативные имена полей внешней таблицы
};

class MyField{
public:
    MyField ()
    {
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
    QString alterfield;                                                         /// альтернативное поле (отображаемое вместо текущего поля)
    QString realtype;                                                           ///
    bool    isEditable;                                                         /// признак редактируемости
    bool    isVisible;                                                          /// признак видимости в вьювере
    bool    isForeign;                                                          /// признак поля-внешнего ключа
    bool    isPrimary;                                                          /// признак поля-первичного ключа
    MyDataReference reference;                                                  /// параметр - ссылка на внешнюю таблицу
    MyDataOfKey     dataofkey;                                                  /// данные внешнего ключа (если поле-внешний ключ)

    QStringList variantsforenum;                                                /// варианты значений для поля с realtype = enum
};

#endif // MYSQLFIELD_H
