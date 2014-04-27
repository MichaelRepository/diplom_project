#include "myinfoscheme.h"
#include "dbmessdlg.h"

MyInfoScheme::MyInfoScheme(const QString &connectname, QObject *parent) :
    QObject(parent)
{
    connectionname = connectname;
    QSqlDatabase db = QSqlDatabase::database(connectname);
    if(!db.open())
    {
        sqlerror = db.lastError();
        error = true;
        return;
    }
    dbname = db.databaseName();
    query  = new QSqlQuery(db);
    error  = false;
}

bool MyInfoScheme::getDataForFields(QList<MyField> &fields)
{
    if(fields.isEmpty() ) return false;
    QString querytext =
            "SELECT "
                "information_schema.COLUMNS.TABLE_NAME, "
                "information_schema.COLUMNS.COLUMN_NAME, "
                "information_schema.COLUMNS.COLUMN_COMMENT,"
                "information_schema.COLUMNS.DATA_TYPE, "
                "information_schema.COLUMNS.COLUMN_TYPE, "
                +dbname+".extenddataforfield.alterfield, "
                +dbname+".extenddataforfield.regex "
            "FROM information_schema.COLUMNS "
                "LEFT JOIN "+dbname+".extenddataforfield "
                    " ON ("+dbname+".extenddataforfield.tablename = information_schema.COLUMNS.TABLE_NAME"
                    " AND " +dbname+".extenddataforfield.fieldname = information_schema.COLUMNS.COLUMN_NAME) "
            "WHERE TABLE_SCHEMA = '"+dbname+"' AND (";
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if(itr != fields.begin() ) querytext += " OR ";
        querytext += " (TABLE_NAME = '"  +(*itr).table->name+"'"+
                     " AND COLUMN_NAME = '" +(*itr).name+"' ) ";
    }
    querytext += " )";

    query->prepare(querytext);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return false;
    }
    error = false;
    if(query->size() == 0){return false;}

    while (query->next() )
    {
       QString table = query->value(0).toString();
       QString field = query->value(1).toString();
       int index = indexFieldOf(table, field, fields);
       if(index == -1) return false;
       MyField* curfield = & fields[index];
       curfield->altername    = query->value(2).toString();
       curfield->realtype     = query->value(3).toString().toUpper();
       curfield->alterfield   = query->value(5).toString();
       curfield->validator    = query->value(6).toString();

       if(curfield->realtype == "ENUM")
       {
           QString coltypedata = query->value(4).toString();
           coltypedata = coltypedata.remove(QRegExp("(enum\\()|(\\))|\\'",Qt::CaseInsensitive) );
           //qDebug() << coltypedata;
           curfield->variantsforenum = coltypedata.split(',');
       }
    }

    return true;
}

QString MyInfoScheme::getAlterName(const QString &table, const QString &field)
{
    query->prepare("SELECT COLUMN_COMMENT FROM information_schema.COLUMNS "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? AND COLUMN_NAME = ?");
    query->bindValue(0, dbname);
    query->bindValue(1, table);
    query->bindValue(2, field);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return QString();
    }
    if(query->size() == 0){return QString();}
    query->seek(0);
    return query->value(0).toString();
}

QString MyInfoScheme::getValidator(const QString &table, const QString &field)
{
    query->prepare("SELECT regex FROM extenddataforfield "
                   "WHERE tablename = ? AND fieldname = ? ");
    query->bindValue(0, table);
    query->bindValue(1, field);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return QString();
    }
    if(query->size() == 0){return QString();}
    query->seek(0);
    return query->value(0).toString();
}

QString MyInfoScheme::getAlterField(const QString &table, const QString &field)
{
    query->prepare("SELECT alterfield FROM extenddataforfield "
                   "WHERE tablename = ? AND fieldname = ? ");
    query->bindValue(0, table);
    query->bindValue(1, field);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return QString();
    }
    if(query->size() == 0){return QString();}
    query->seek(0);
    return query->value(0).toString();
}

QString MyInfoScheme::getPrimaryKey(const QString &table)
{
    query->prepare("SELECT COLUMN_NAME FROM information_schema.key_column_usage "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? "
                   "AND REFERENCED_TABLE_NAME IS NULL");
    query->bindValue(0, dbname);
    query->bindValue(1, table);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return QString();
    }
    if(query->size() == 0){return QString();}
    query->seek(0);
    query->value(0).toString();
    return query->value(0).toString();
}

MyDataOfKey MyInfoScheme::getKeyData(const QString &table, const QString &tablerefrence)
{
    MyDataOfKey key;
    query->prepare("SELECT COLUMN_NAME, REFERENCED_COLUMN_NAME "
                   "FROM information_schema.key_column_usage "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? AND REFERENCED_TABLE_NAME = ?");
    query->bindValue(0, dbname);
    query->bindValue(1, table);
    query->bindValue(2, tablerefrence);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return key;
    }
    if(query->size() == 0){return key;}
    query->seek(0);
    key.table = table;
    key.referencedtable = tablerefrence;
    key.foreignkey = query->value(0).toString();
    key.primarykey = query->value(1).toString();
    return key;
}

void MyInfoScheme::getForeignKeys(const QString &table, QVector<MyDataOfKey> &keysdata)
{
    query->prepare("SELECT REFERENCED_TABLE_NAME, COLUMN_NAME, REFERENCED_COLUMN_NAME "
                   "FROM information_schema.key_column_usage "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ?");
    query->bindValue(0, dbname);
    query->bindValue(1, table);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return ;
    }
    if(query->size() == 0){ return ;}
    MyDataOfKey key;
    while(query->next())
    {
        key.table           = table;
        key.referencedtable = query->value(0).toString();
        key.foreignkey      = query->value(1).toString();
        key.primarykey      = query->value(2).toString();
        keysdata.append(key);
    }
}

bool MyInfoScheme::isForeignKey(const QString &table, const QString &field)
{
    query->prepare("SELECT REFERENCED_TABLE_NAME "
                   "FROM information_schema.key_column_usage "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? AND COLUMN_NAME = ?");
    query->bindValue(0, dbname);
    query->bindValue(1, table );
    query->bindValue(2, field );
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return false;
    }
    if(query->size() == 0){return false;}
    return true;
}

void MyInfoScheme::getAlterNamesFieldOfTable(const QString &table, QStringList &alternames)
{
    query->prepare("SELECT COLUMN_COMMENT FROM information_schema.COLUMNS "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? "
                   "GROUP BY ORDINAL_POSITION");
    query->bindValue(0, dbname);
    query->bindValue(1, table);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return ;
    }
    if(query->size() == 0){return ;}
    while(query->next())
    {
        alternames.append(query->value(0).toString());
    }
}

bool MyInfoScheme::tableIsValid(QString table)
{
    query->prepare("SELECT TABLE_TYPE FROM information_schema.TABLES "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? ");
    query->bindValue(0, dbname);
    query->bindValue(1, table);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return false;
    }
    if(query->size() == 0){return false;}
    return true;
}

bool MyInfoScheme::isError()
{
    return error;
}

QSqlError MyInfoScheme::lastQueryError()
{
    return sqlerror;
}

QString MyInfoScheme::getAlterNameForTable(const QString &tablename)
{

    query->prepare("SELECT TABLE_COMMENT FROM information_schema.TABLES "
                   "WHERE TABLE_SCHEMA = ? AND TABLE_NAME = ? ");
    query->bindValue(0, dbname);
    query->bindValue(1, tablename);
    if(!query->exec() )
    {
        sqlerror = query->lastError();
        error = true;
        return QString();
    }
    if(query->size() == 0) return QString();
    query->first();
    return query->value(0).toString();

}

int MyInfoScheme::indexFieldOf(QString table, QString field,
                               QList<MyField> &fields) const
{
    QList<MyField>::const_iterator itr;
    int index = 0;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).table->name == table && (*itr).name == field) return index;
        ++ index;
    }
    return -1;
}
