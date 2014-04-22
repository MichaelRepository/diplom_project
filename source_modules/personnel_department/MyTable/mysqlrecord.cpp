#include "mysqlrecord.h"

MySqlRecord::MySqlRecord()
{
}

void MySqlRecord::append(const MyField *field)
{
    fields.append(field);
    values.append(QVariant());
}

void MySqlRecord::append(const MyField *field, QVariant value)
{
    fields.append(field);
    values.append(value);
}

void MySqlRecord::replace(int pos, const MyField *field)
{
    if(pos >= 0 && pos <fields.size())
    {
        fields[pos] = field;
        values[pos].clear();
        if(altervalues.contains(pos)) altervalues.remove(pos);
    }
}

void MySqlRecord::insert(int pos, const MyField *field)
{
    fields.insert(pos, field);
}

void MySqlRecord::remove(int pos)
{
    if(pos >= 0 && pos <fields.size())
    {
        fields.removeAt(pos);
        values.removeAt(pos);
        if(altervalues.contains(pos)) altervalues.remove(pos);
    }
}

QVariant MySqlRecord::value(int i) const
{
    if(i >= 0 && i <fields.size())
        return values[i];
    return QVariant();
}

QVariant MySqlRecord::value(const QString &name) const
{
    int pos = indexOf(name);
    if(pos >= 0 && pos <fields.size())
        return values[pos];
    return QVariant();
}

bool MySqlRecord::isVisibleField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->isVisible;
    return false;
}

bool MySqlRecord::isEditableField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->isEditable;
    return false;
}

bool MySqlRecord::isPrimary(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->isPrimary;
    return false;
}

bool MySqlRecord::isForeign(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->isForeign;
    return false;
}

QString MySqlRecord::alterNameOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->altername;
    return QString();
}

QString MySqlRecord::validatorOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->validator;
    return QString();
}

QString MySqlRecord::realTypeOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->realtype;
    return QString();
}

QString MySqlRecord::alterField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->alterfield;
    return QString();
}

QVariant MySqlRecord::alterData(int i)
{
    if(i >= 0 && i <fields.size())
    if(altervalues.contains(i) ) return altervalues[i];
    return QVariant();
}

MyDataReference MySqlRecord::referenceDataOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i]->reference;
    return MyDataReference();
}

void MySqlRecord::setValue(int i, const QVariant &val)
{
    if(i >= 0 && i <fields.size())
        values[i] = val;
}

void MySqlRecord::setAlterData(int i, QVariant data)
{
    if(i >= 0 && i <fields.size())
    if(altervalues.contains(i)) altervalues[i] = data;
    else altervalues.insert(i,data);
}

void MySqlRecord::setValue(const QString &name, const QVariant &val)
{
        setValue(indexOf(name), val);
}

int MySqlRecord::indexOf(const QString &name) const
{
    QString nm = name.toUpper();
    for (int i = 0; i < count(); ++i) {
        if (fields.at(i)->name.toUpper() == nm)
        return i;
    }
    return -1;
}

QString MySqlRecord::fieldName(int i) const
{
    if(i >= 0 && i <fields.size())
        return fields[i]->name;
    return QString();
}

const MyField *MySqlRecord::field(int i) const
{
    if(i >= 0 && i <fields.size())
        return fields[i];
    return 0;
}

const MyField *MySqlRecord::field(const QString &name) const
{
    int pos = indexOf(name);
    if(pos >= 0 && pos <fields.size())
        return fields[pos];
    return 0;
}

bool MySqlRecord::isEmpty() const
{
    return fields.isEmpty();
}

bool MySqlRecord::contains(const QString &name) const
{
    return (indexOf(name)>-1 );
}

void MySqlRecord::clear()
{
    fields.clear();
    values.clear();
    altervalues.clear();
}

int MySqlRecord::count() const
{
    return fields.size();
}
