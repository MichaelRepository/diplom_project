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
