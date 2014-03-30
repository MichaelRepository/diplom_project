#include "mysqlrecord.h"

MySqlRecord::MySqlRecord()
{
}

void MySqlRecord::append(const MySqlField &field)
{
    fields.append(field);
}

void MySqlRecord::replace(int pos, const MySqlField &field)
{
    if(pos >= 0 && pos <fields.size())
        fields[pos] = field;
}

void MySqlRecord::insert(int pos, const MySqlField &field)
{
    fields.insert(pos, field);
}

void MySqlRecord::remove(int pos)
{
    if(pos >= 0 && pos <fields.size())
        fields.removeAt(pos);
}

QVariant MySqlRecord::value(int i) const
{
    if(i >= 0 && i <fields.size())
        return fields[i].value();
    return QVariant();
}

QVariant MySqlRecord::value(const QString &name) const
{
    int pos = indexOf(name);
    if(pos >= 0 && pos <fields.size())
        return fields[pos].value();
    return QVariant();
}

bool MySqlRecord::isVisibleField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].isVisible();
    return false;
}

bool MySqlRecord::isEditableField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].isEditable();
    return false;
}

bool MySqlRecord::isReference(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].isReference();
    return false;
}

QString MySqlRecord::alterNameOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].alterName();
    return QString();
}

QString MySqlRecord::validatorOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].validator();
    return QString();
}

QString MySqlRecord::realTypeOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].realType();
    return QString();
}

QString MySqlRecord::alterField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].alterField();
    return QString();
}

QVariant MySqlRecord::alterData(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].alterData();
    return QVariant();
}

MyDataReference MySqlRecord::referenceDataOfField(int i)
{
    if(i >= 0 && i <fields.size())
        return fields[i].referenceData();
    return MyDataReference();
}

void MySqlRecord::setValue(int i, const QVariant &val)
{
    if(i >= 0 && i <fields.size())
        fields[i].setValue(val);
}

void MySqlRecord::setAlterName(int i, const QString &altername)
{
    if(i >= 0 && i <fields.size())
        fields[i].setAlterName(altername);
}

void MySqlRecord::setValidator(int i, QString valid)
{
    if(i >= 0 && i <fields.size())
        fields[i].setValidator(valid);
}

void MySqlRecord::setEditable(int i, bool editable)
{
    if(i >= 0 && i <fields.size())
        fields[i].setEditable(editable);
}

void MySqlRecord::setVisible(int i, bool visible)
{
    if(i >= 0 && i <fields.size())
        fields[i].setVisible(visible);
}

void MySqlRecord::setRealType(int i, QString type)
{
    if(i >= 0 && i <fields.size())
        fields[i].setRealType(type);
}

void MySqlRecord::setAlterField(int i, QString alterfeld)
{
    if(i >= 0 && i <fields.size())
        fields[i].setAlterField(alterfeld);
}

void MySqlRecord::setAlterData(int i, QVariant data)
{
    if(i >= 0 && i <fields.size())
        fields[i].setAlterData(data);
}

void MySqlRecord::setValue(const QString &name, const QVariant &val)
{
        setValue(indexOf(name), val);
}

void MySqlRecord::setAlterName(const QString& name, const QString &altername)
{
    setAlterName(indexOf(name), altername);
}

void MySqlRecord::setValidator(const QString& name, QString valid)
{
    setValidator(indexOf(name), valid);
}

void MySqlRecord::setEditable(const QString& name, bool editable)
{
    setEditable(indexOf(name), editable);
}

void MySqlRecord::setVisible(const QString& name, bool visible)
{
    setVisible(indexOf(name), visible);
}

void MySqlRecord::setRealType(const QString& name, QString type)
{
    setRealType(indexOf(name), type);
}

void MySqlRecord::setAlterField(const QString &field, QString alterfeld)
{
    setAlterField(indexOf(field), alterfeld);
}

int MySqlRecord::indexOf(const QString &name) const
{
    QString nm = name.toUpper();
    for (int i = 0; i < count(); ++i) {
        if (fields.at(i).name().toUpper() == nm)
        return i;
    }
    return -1;
}

QString MySqlRecord::fieldName(int i) const
{
    if(i >= 0 && i <fields.size())
        return fields[i].name();
    return QString();
}

MySqlField MySqlRecord::field(int i) const
{
    if(i >= 0 && i <fields.size())
        return fields[i];
    return MySqlField();
}

MySqlField MySqlRecord::field(const QString &name) const
{
    int pos = indexOf(name);
    if(pos >= 0 && pos <fields.size())
        return fields[pos];
    return MySqlField();
}

bool MySqlRecord::isEmpty() const
{
    return fields.isEmpty();
}

bool MySqlRecord::contains(const QString &name) const
{
    return fields.contains(name)>=0;
}

void MySqlRecord::clear()
{
    fields.clear();
}

int MySqlRecord::count() const
{
    return fields.size();
}





