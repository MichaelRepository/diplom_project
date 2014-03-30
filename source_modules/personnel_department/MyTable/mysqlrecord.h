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

    void append(const MySqlField& field);
    void replace(int pos, const MySqlField& field);
    void insert(int pos, const MySqlField& field);
    void remove(int pos);

    QVariant value(int i) const;
    QVariant value(const QString& name) const;
    bool isVisibleField(int i);
    bool isEditableField(int i);
    bool isReference (int i);
    QString alterNameOfField(int i);
    QString validatorOfField(int i);
    QString realTypeOfField(int i);
    QString alterField(int i);
    QVariant alterData(int i);
    MyDataReference referenceDataOfField(int i);

    void setValue    (int i, const QVariant& val);
    void setAlterName(int i, const QString& altername);
    void setValidator(int i, QString valid);
    void setEditable (int i, bool  editable);
    void setVisible  (int i, bool  visible);
    void setRealType (int i, QString type);
    void setAlterField (int i, QString alterfeld);
    void setAlterData  (int i, QVariant data);

    void setValue    (const QString& name, const QVariant& val);
    void setAlterName(const QString& name, const QString& altername);
    void setValidator(const QString& name, QString valid);
    void setEditable (const QString& name, bool  editable);
    void setVisible  (const QString& name, bool  visible);
    void setRealType (const QString& name, QString type);
    void setAlterField (const QString& field, QString alterfeld);

    int indexOf(const QString &name) const;
    QString fieldName(int i) const;

    MySqlField field(int i) const;
    MySqlField field(const QString &name) const;

    bool isEmpty() const;
    bool contains(const QString& name) const;
    void clear();
    int count() const;

private:
    QList<MySqlField> fields;
};

#endif // MYSQLRECORD_H
