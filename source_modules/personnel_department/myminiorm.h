#ifndef MYMINIORM_H
#define MYMINIORM_H

#include <QObject>
#include <QTableView>
#include <QtSql>

/**
    Простейшая MyMiniORM
**/

class MyForeignKeyData
{
public:
    explicit MyForeignKeyData() {isValid = false;}

    QString mastertable;                                                        /// главная таблица
    QString slavetable;                                                         /// подчиненная табоица
    QString keyfield;                                                           /// ключевое поле главной таблицы
    QString linkablefield;                                                      /// поле текущейтаблицы которое ссылается на ключ
    bool isValid;                                                               /// валидность объекта
};

class MyForeignKeys:public QObject                                              /// список всех ключей
{
    Q_OBJECT
public:
    explicit MyForeignKeys(QObject *parent = 0): QObject(parent){}
    ~MyForeignKeys(){
        /// ОЧИСТИТЬ ВСЕ СПИСКИ!!!!
    }

    void addForeignKey(QString mastertable, QString slavetable,                 /// добавление ключа
                       QString keyfield,    QString linkablefield)
    {
        if(mastertables.contains(mastertable) &&
           mastertables[mastertable].contains(slavetable))
        {
                MyForeignKeyData* keydata = mastertables[mastertable][slavetable];
                keydata->keyfield      = keyfield;
                keydata->linkablefield = linkablefield;
        }
        else
        {
            MyForeignKeyData keydata;
            keydata.keyfield      = keyfield;
            keydata.linkablefield = linkablefield;
            keydata.mastertable   = mastertable;
            keydata.slavetable    = slavetable;
            keysdata.append(keydata);


            if(mastertables.contains(mastertable))
                mastertables[mastertable].insert(slavetable,&keysdata.last());
            else
            {
                QMap<QString,MyForeignKeyData*> masterdata;
                masterdata.insert(slavetable,&keysdata.last());
                mastertables.insert(mastertable,masterdata);
            }

            if(slavetables.contains(slavetable))
                slavetables[slavetable].insert(mastertable,&keysdata.last());
            else
            {
                QMap<QString,MyForeignKeyData*> slavedata;
                slavedata.insert(mastertable,&keysdata.last());
                slavetables.insert(slavetable,slavedata);
            }

        }
    }

    QStringList getMasterTables(QString table)                                  /// получить список (мастер) таблиц на которые ссылается текущая
    {
        if(slavetables.contains(table))
            return slavetables[table].keys();
        else
            return QStringList();
    }

    QStringList getSlaveTables(QString table)                                   /// получить список (подчиненных) таблиц, ссылающихся на текщую
    {
        if(mastertables.contains(table))
            return mastertables[table].keys();
        else
            return QStringList();
    }

    QString getSectionWhere(QStringList* tables)                                /// получить зависимость таблиц tables в виде участка "WHERE" запроса
    {
        QString str = "";
        QTextStream stream(&str);
        if(tables->size() == 0) return str;

        QStringList::iterator itrt;
        for(itrt = tables->begin(); itrt != tables->end(); ++itrt)
        {
            QStringList::iterator subitrt;
            for(subitrt = tables->begin(); subitrt != tables->end(); ++subitrt)
            {
                if(slavetables.contains(*itrt) &&
                   slavetables[*itrt].contains(*subitrt))
                {
                    MyForeignKeyData *key = slavetables[*itrt][*subitrt];
                    if(itrt != tables->begin()) stream << " AND ";
                    stream << *itrt    << "." << key->linkablefield << " = "
                           << *subitrt << "." << key->keyfield;
                }
            }
        }

        return str;
    }

    /*MyForeignKeyData getKeyData(QString slavetable, QString mastertable)        /// получить парметр ключа, связыающий две таблицы
    {
        if(foreignkey.contains(slavetable) &&
           foreignkey[slavetable].contains(mastertable))
            return foreignkey[slavetable][mastertable];
        else
            return MyForeignKeyData();
    }*/

private:
    QList<MyForeignKeyData> keysdata;                                           /// данные по ключам
    QMap<QString, QMap<QString,MyForeignKeyData*> > mastertables;               /// ассоциативная таблица для мастер (мастер,(слейв,ключ))
    QMap<QString, QMap<QString,MyForeignKeyData*> > slavetables;                /// ассоциативная таблица для слейв  (слейв,(мастер,ключ))
};

class MyTableData:public QObject                                                /// метаобъект - Данные Таблицы
{
    Q_OBJECT
public:
    explicit MyTableData(QObject *parent = 0): QObject(parent){}
    ~MyTableData(){
        /// ОЧИСТКА ДАННЫХ
    }

    void initTableData(QString connectname,                                     /// инициализация данных
                       MyForeignKeys* keys)
    {
        clearTable();
        connectionname  = connectname;
        foreignkeys     = keys;
    }

    void addField(QString name, QString altername,                              /// добавить поле для таблицы
                  QString validatorstr, MyTableData* link)
    {
        /// ТУТОЧКИ !!!
    }

    void createTable()                                                          /// создать/пересоздать таблицу
    {
        QSqlDatabase db = QSqlDatabase::database(connectionname);
        if(!db.open()){
            /// ОТОБРАЗИТЬ СООБЩЕНИЕ ОБ ОШИБКЕ
            return;
        }
        QString str = createQueryText();
        querymodel.setQuery(str,db);
        if(querymodel.lastError().isValid()){
            /// ОТОБРАЗИТЬ СООБЩЕНИЕ ОБ ОШИБКЕ
        }
    }

    void setTableViewer(QTableView* viewer)                                     /// подключиться к вьюверу
    {
        viewer->setEditTriggers(QAbstractItemView::NoEditTriggers);             /// запретить редактирвоание
        viewer->setModel(&querymodel);
        /// ПЕРЕИМЕНОВАТЬ ПОЛЯ В ШАПКЕ ТАБЛИЦЫ
    }

    void clearTable()                                                           /// очистить все списки таблицы
    {
        querymodel.clear();
        alternames.clear();
        regexpvalidators.clear();
    }

protected:
    QString createQueryText()                                                   /// создать текст запроса
    {
        getTableList();                                                         /// сформировать список таблиц
        QString sectionwhere =  foreignkeys->getSectionWhere(&tablelist);       /// получить участок "WHERE" запроса

        QString querytext;
        QTextStream stream(&querytext);

        stream << "SELECT ";
        /// формируется участок запроса: список полей
        QList<QPair<QString,QString> >::iterator itrf;
        for(itrf = fieldslist.begin(); itrf != fieldslist.end();++itrf)
        {
            if(itrf != fieldslist.begin()) stream << ", ";
            if((*itrf).first == "***")                                          /// если идет ссылка на таблицу
                stream << "'***' AS " << (*itrf).second;
            else
                stream << (*itrf).first << "." << (*itrf).second;
        }

        stream << " FROM ";
        /// формируется участок запроса: список таблиц
        QStringList::iterator itrt;
        for(itrt = tablelist.begin(); itrt != tablelist.end(); ++itrt)
        {
            if(itrt != tablelist.begin()) stream << ", ";
            stream << (*itrt);
        }

        /// формируется участок запроса: WHERE
        if(sectionwhere != "") stream << " WHERE " << sectionwhere;

        qDebug() << querytext;
        return querytext;
    }

    void getTableList()                                                         /// получить список таблиц
    {
        tablelist.clear();
        QList<QPair<QString,QString> >::iterator itrf;
        for(itrf = fieldslist.begin(); itrf != fieldslist.end(); ++itrf){
            if(!tablelist.contains((*itrf).first) && (*itrf).first != "***")    /// не таблица, а ссылка на таблицу
                tablelist << (*itrf).first;
        }
    }

    QString                 connectionname;                                     /// имя подключения
    QString                 querytext;                                          /// запрос сформированный АВТОМАТИЧЕСКИ

    QStringList             tablelist;                                          /// список таблиц сформированный АВТОМАТИЧЕСКИ
    QList<QPair<QString,
                QString> >  fieldslist;                                         /// список полей      (может быть пустым)
    QMap<QString,QString>   alternames;                                         /// (таблица.поле,псевдоним) список альтернативных имен полей
    QStringList             regexpvalidators;                                   /// список значений для валидаторов
    MyForeignKeys*          foreignkeys;                                        /// указатель на глобальный список ключей БД

    QMap<QString, MyTableData*> linkstotables;                                  /// атрибуты-ссылки на таблицы (поле-атрибут, указатель на таблицу)

    QSqlQueryModel          querymodel;                                         /// модель данных
    /// ФИЛЬТР!!!!

};

#endif // MYMINIORM_H
