#ifndef MYSQLRECORD_H
#define MYSQLRECORD_H

#include <QVector>
#include "mysqlfield.h"
/*
   моя версия класса SqlRecord
*/

class MySqlRecord : public QObject
{
    Q_OBJECT
public:
    MySqlRecord();

    void append (const MyField* field);
    void append (const MyField* field, QVariant value);
    void replace(int pos, const MyField* field);
    void insert (int pos, const MyField* field);
    void remove (int pos);

    QVariant value(int i) const;
    QVariant value(const QString& name) const;
    bool isVisibleField (int i);
    bool isEditableField(int i);
    bool isPrimary    (int i);
    bool isForeign    (int i);
    QString alterNameOfField(int i);
    QString validatorOfField(int i);
    QString realTypeOfField (int i);
    QString alterField(int i);
    QVariant alterData(int i);
    MyDataReference referenceDataOfField(int i);

    void setValue       (int i, const QVariant& val);
    void setValue       (const QString& name, const QVariant& val);
    void setAlterData   (int i, QVariant data);

    int indexOf(const QString &name) const;
    QString fieldName(int i) const;

    const MyField* field(int i) const;
    const MyField* field(const QString &name) const;

    bool isEmpty() const;
    bool contains(const QString& name) const;
    void clear();
    int  count() const;

private:
    QList<const MyField*> fields;
    QList<QVariant>       values;
    QMap<int,QVariant>    altervalues;
};

#endif // MYSQLRECORD_H
