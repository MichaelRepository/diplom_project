#ifndef MYSQLFIELD_H
#define MYSQLFIELD_H

#include <QSqlField>

class MyDataReference{ /// класс для хранения данных поля - ссылки
public:
    QString     sqltext;                                                        /// запрос на внешнюю таблицу
    QStringList alternames;                                                     /// алтернативные имена внешней таблицы
};


class MySqlField : public QSqlField
{
public:
    MySqlField(const QString & fieldName = QString(),
               QVariant::Type type = QVariant::Invalid,
               const QString & altername = QString(),
               const QString & validator = QString(),
               const QString & realtype  = QString(),
               const MyDataReference &referencedata = MyDataReference(),
               bool visible  = true,
               bool editable = true):
    QSqlField(fieldName,type)
    {
        fieldaltername = altername;
        fieldvalidator = validator;
        fieldrealtype  = realtype;
        fieldvisible   = visible;
        fieldeditable  = editable;
        if(referencedata.sqltext.size() > 0)
        {
            reference      = referencedata;
            fieldreference = true;
        }
        else
            fieldreference = false;
    }

    MySqlField(const QSqlField &field)
    {
        this->setValue(field.value());
        this->setName(field.name());
        this->setType(field.type());
        this->setAutoValue(field.isAutoValue());
        this->setDefaultValue(field.defaultValue());
        this->setGenerated(field.isGenerated());
        this->setLength(field.length());
        fieldreference = false;
    }

    void clear(){
        QSqlField::clear();
        fieldaltername = "";
        fieldvalidator = "";
        fieldrealtype  = "";
        fieldvisible   = true;
        fieldeditable  = true;
        fieldreference = false;
    }

    void setAlterName(QString alter) {fieldaltername = alter;}
    void setValidator(QString valid) {fieldvalidator = valid;}
    void setEditable (bool  editable){fieldeditable  = editable;}
    void setVisible  (bool  visible) {fieldvisible   = visible;}
    void setRealType (QString type)  {fieldrealtype  = type;}
    void setReference(const MyDataReference& referencedata)
    {
        if(referencedata.alternames.size() > 0)
        {
            reference = referencedata;
            fieldreference = true;
        }
    }
    void setAlterField(QString field){alterfield = field;}
    void setAlterData (QVariant data){alterdata = data;}


    bool isVisible()     {return fieldvisible;}
    bool isEditable()    {return fieldeditable;}
    bool isReference()   {return fieldreference;}
    QString  alterName() {return fieldaltername;}
    QString  validator() {return fieldvalidator;}
    QString  realType()  {return fieldrealtype;}
    QString  alterField(){return alterfield;}
    QVariant alterData (){return alterdata;}
    MyDataReference referenceData() {return reference;}

private:
    QString fieldaltername;  /// алтернативное имя поля
    QString fieldvalidator;  /// валидатор для редактора поля
    QString fieldrealtype;   /// реальный тип данных по стандарту СУБД
    bool    fieldeditable;   /// видим в редакторе
    bool    fieldvisible;    /// видим в tableviewer
    MyDataReference reference; /// данные ссылки на внешнюю таблицу
    bool    fieldreference;  /// признак того, что поле - ссылка
    /// поле, значение и имя которого будут выводиться в !РЕДАКТОРЕ! вместо текущего
    /// должно быть указано поле из СЛИНКОВАННОЙ таблицы, а не из текущей!
    QString  alterfield;
    QVariant alterdata;
};

#endif // MYSQLFIELD_H
