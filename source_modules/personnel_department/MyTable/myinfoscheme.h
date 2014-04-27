#ifndef MYINFOSCHEME_H
#define MYINFOSCHEME_H

#include <QObject>
#include <QtSql>
#include <QDebug>

#include "mysqlfield.h"

class MyInfoScheme: QObject
{
    Q_OBJECT

public:
    MyInfoScheme(const QString &connectname,   QObject *parent = 0);
    bool getDataForFields(QList<MyField> &fields);
    QString getAlterName (const QString &table, const QString &field);          /// получить альтернативное имя поля
    QString getValidator (const QString &table, const QString &field);
    QString getAlterField(const QString &table, const QString &field);          /// получить имя альтернативного поля (значение и имя которого будут выводится в РЕДАКТОРЕ!)
    QString getPrimaryKey(const QString &table);                                /// получить первыичный ключ таблицы
    MyDataOfKey getKeyData(const QString &table, const QString &tablerefrence); /// получить данные ключа, связывающего таблицы
    void getForeignKeys(const QString &table, QVector<MyDataOfKey>& keysdata);  /// получить данные всех внешних ключей текущей таблицы
    bool isForeignKey(const QString &table, const QString &field);              /// проверить является поле данной таблицы внешним ключем
    void getAlterNamesFieldOfTable(const QString &table, QStringList &alternames);   /// получить список альтернативных имен таблицы
    bool tableIsValid(QString table);
    bool isError();
    QSqlError lastQueryError();

    QString getAlterNameForTable(const QString& tablename);

private:

    int indexFieldOf(QString table, QString field, QList<MyField> &fields) const;

    bool error;
    QSqlError  sqlerror;
    QSqlQuery* query;
    QString    connectionname;
    QString    dbname;
};

#endif // MYINFOSCHEME_H
