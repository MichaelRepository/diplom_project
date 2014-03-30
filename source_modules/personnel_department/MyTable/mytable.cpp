#include "mytable.h"

MyTable::MyTable(MyInfoScheme *metadatascheme, QString connectname)
{
    currentrow = -1;
    filtered   = false;
    freeFilter = "";
    connectionname = connectname;
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    sqlerror = db.lastError();
    if(!db.open())
    {
        return;
    }
    tablequery = QSqlQuery(db);
    metadata = metadatascheme;

}

void MyTable::appendField(QString field, QString table, bool editable, bool visible)
{
    if(fieldlist.contains(field)) return; /// есть такое поле => проигнорировать
    fieldlist.append(field);
    filters.append("");

    if(table != "")
    {
        tablenamelist.append(table);
        if(!subtable.contains(table)) subtable.append(table);                   /// формируется список субтаблиц
    }
    else
        tablenamelist.append(tablename);

    flagseditable.append(editable);
    flagsvisible.append(visible);
    /// индексация отображаемых столбцов
    /// index(num, real_num) num - это номер по которому на неё ссылаются
    /// real_num - номер, который она занимает в таблице
    if(visible) indexesofvisiblecolumns.insert(indexesofvisiblecolumns.size(),
                                               fieldlist.size()-1);

}

bool MyTable::initializeTable()
{
    /// проверка источника метаданных
    if(metadata->isError()) return false;
    /// проверка валидности таблицы
    if(!metadata->tableIsValid(tablename)) return false;

    /// очистка данных
    if(fieldlist.size()      == 0) return false;
    if(alternamelist.size()   > 0) alternamelist.clear();
    if(validatorlist.size()   > 0) validatorlist.clear();
    if(editablerecord.count() > 0) editablerecord.clear();
    if(references.count()     > 0) references.clear();
    lastquerytext = "";
    filtered = false;

    if(!preparePrimaryKeysData()) return false;

    /// сформировать SELECT запрос
    if(!generateSelectQuery()) return false;
    /// получить список альтернативных имен для каждого поля +
    /// валидатор + альтернативное поле
    QStringList::iterator itrf = fieldlist.begin();
    QStringList::iterator itrt = tablenamelist.begin();
    while (itrf != fieldlist.end())
    {
        QString altername = metadata->getAlterName(*itrt,*itrf);
        if(metadata->isError()) return false;
        QString validator = metadata->getValidator(*itrt,*itrf);
        if(metadata->isError()) return false;
        QString alterfield = metadata->getAlterField(*itrt,*itrf);
        if(metadata->isError()) return false;
        alternamelist << altername;
        validatorlist << validator;
        alterfields   << alterfield;
        ++itrf;
        ++itrt;
    }
    /// генерация структуры для update и insert запросов
    generateUpdateQuery();
    /// генерация запросов для полей - ссылок
    /// поля, которые являются ссылками на внешнюю таблицу в редакторе должны
    /// отображать таблицу на которую они ссылаются
    if(!prepareForeignKeysData()) return false;

    if(!updateData()) return false;

    return true;
}

void MyTable::setFilterForField(QString field, QString filter)
{
    int index = fieldlist.indexOf(field);
    if(index != -1)
        filters[index] = filter;
}

void MyTable::setFilterForTable(QString str)
{
    freeFilter = str;
}

void MyTable::setFilterActivity(bool activ)
{
    filtered = activ;
}

void MyTable::setCurrentRow(int index){
    currentrow = index;
}

int MyTable::getRecordsCount()
{
    return tablequery.size();
}

int MyTable::getFieldsCount()
{
    /// вернуть число видимых полей (столбцов)
    return indexesofvisiblecolumns.size();
}

bool MyTable::updateData()
{
/// основной запрос
    QString querytext = lastquerytext;
    if(filtered)
    {
        if(freeFilter.contains("WHERE",Qt::CaseInsensitive))
            freeFilter.remove("WHERE",Qt::CaseInsensitive);

        QString filtersection;
        if(!querytext.contains("WHERE"))
            filtersection += " WHERE ";
        else
            filtersection += " AND ";

        QStringList::iterator itr;
        int i = 0;
        int filteredfieldcount = 0;
        for(itr = filters.begin(); itr != filters.end(); ++itr)
        {
            if((*itr).size() > 0)
            {
                if(filteredfieldcount > 0) filtersection += " AND ";
                QString field = fieldlist[i];
                QString value = "'"+(*itr)+"'";
                filtersection += field+" = "+value;
                ++filteredfieldcount;
            }
            ++i;
        }


        if(freeFilter.size() > 0)
        {
            if(filteredfieldcount > 0) filtersection += " AND ";
            filtersection += freeFilter;
        }

        if(lastquerytext.contains("ORDER BY",Qt::CaseInsensitive))
        {
            int pos = lastquerytext.indexOf("ORDER BY",Qt::CaseInsensitive);
            querytext.insert(pos, filtersection+" ");
        }
        else
            querytext += filtersection;
    }

qDebug() << querytext;

    tablequery.prepare(querytext);
    if(!tablequery.exec() )
    {
        sqlerror = tablequery.lastError();
        return false;
    }
    return true;
}

bool MyTable::setUpdateDataFields()
{
    if(fieldsforquery.size() == 0 || editablerecord.count() == 0) return false;
    if(!tablequery.seek(currentrow)) return false;
    QMap<QString,QStringList>::iterator itr;
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()) return false;
    QSqlQuery query(db);
    int index = 0;
    for(itr = fieldsforquery.begin(); itr != fieldsforquery.end(); ++itr)
    {
        QString querytext = "UPDATE "+itr.key()+" SET ";
        QStringList::iterator itrl;
        for(itrl = (*itr).begin(); itrl != (*itr).end(); ++itrl)
        {
            if(itrl != (*itr).begin()) querytext += " , ";
            querytext += *itrl + " = '" + editablerecord.value(*itrl).toString() + "' ";
        }
        QString key = primarykeys[index];
        querytext += " WHERE " + key + " = "+ tablequery.value(key).toString();

        db.transaction();
        if(!query.exec(querytext))
        {
            sqlerror = query.lastError();
            db.rollback();
            return false;
        }
        db.commit();


        ++index;
    }
    return true;
}

bool MyTable::setInsertDataFields()
{
    if(fieldsforquery.size() == 0 || editablerecord.count() == 0) return false;
    if(!tablequery.seek(currentrow)) return false;
    QMap<QString,QStringList>::iterator itr;
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()) return false;
    QSqlQuery query(db);
    int index = 0;
    for(itr = fieldsforquery.begin(); itr != fieldsforquery.end(); ++itr)
    {
        QString querytext = "INSERT INTO "+itr.key();
        QString fieldssection;
        QString valuessection;
        QStringList::iterator itrl;
        for(itrl = (*itr).begin(); itrl != (*itr).end(); ++itrl)
        {
            if(itrl != (*itr).begin())
            {
                fieldssection += " , ";
                valuessection += " , ";
            }
            fieldssection += *itrl;
            valuessection += "'" + editablerecord.value(*itrl).toString() + "' ";
        }

        querytext += " (" + fieldssection + ") VALUES (" + valuessection + ") ";

        db.transaction();
        if(!query.exec(querytext))
        {
            sqlerror = query.lastError();
            db.rollback();
            return false;
        }
        db.commit();

        ++index;
    }
    return true;
}

bool MyTable::isFiltered()
{
    return filtered;
}

int MyTable::searchRecordByField(QString field, QVariant value)
{
    if(!fieldlist.contains(field)) return -1;
    for(int i = 0; i < tablequery.size(); ++i)
    {
        tablequery.seek(i);
        if(tablequery.value(field) == value)
            return i;
    }
    return -1;
}

int MyTable::getCurrentRowIndex()
{
    return currentrow;
}

MySqlRecord *MyTable::getRecord(int row)
{
    /// сформировать данные для копии запрашиваемой строки
    if(!tablequery.seek(row)) return 0;
    currentrow = row;                                                           /// автоматом текущая строка приравнивается значению row
    editablerecord.clear();
    QSqlRecord record = tablequery.record();
    for(int i = 0; i < fieldlist.size(); ++i)
    {
        if(flagseditable[i])
        {
            MySqlField field(record.field(i));
            field.setAlterName(alternamelist[i]);
            field.setValidator(validatorlist[i]);
            field.setEditable (flagseditable[i]);
            field.setVisible  (flagsvisible [i]);
            if(references.contains(i))
                field.setReference(references[i]);
            if(alterfields[i].size() > 0)
                field.setAlterField(alterfields[i]);
            editablerecord.append(field);
        }
    }
    qDebug() << editablerecord.fieldName(0);
    return &editablerecord;
}

QSqlError MyTable::lastSqlError()
{
    return sqlerror;
}

QString MyTable::getFieldAlterName(int field)
{
    if(!indexesofvisiblecolumns.contains(field)) return QString();
    /// получить реальный номер поля
    int realindex = indexesofvisiblecolumns[field];
    return alternamelist[realindex];
}

QString MyTable::getFieldName(int col)
{
    if(!indexesofvisiblecolumns.contains(col)) return QString();
    /// получить реальный номер поля
    int realindex = indexesofvisiblecolumns[col];
        return fieldlist[realindex];
}

QVariant MyTable::getCellValue(int row, int col)
{
    if(!tablequery.seek(row) ||
       !indexesofvisiblecolumns.contains(col)) return QVariant();
    /// получить реальный номер поля
    int realindex = indexesofvisiblecolumns[col];
    return tablequery.value(realindex);
}

QVariant MyTable::getFieldValue(int row, QString field)
{
    int colnum = fieldlist.indexOf(field);
    if(colnum == -1) return QVariant();
    return getCellValue(row, colnum);
}

bool MyTable::generateSelectQuery()
{
    QString str = "SELECT ";

    /// формирование списка полей (таблица.поле)
    QStringList::iterator itr;
    int i = 0;
    for(itr = fieldlist.begin(); itr != fieldlist.end(); ++itr)
    {
        if(itr != fieldlist.begin())  str += ", ";
        str +=  tablenamelist[i] + "." + *itr;
        ++i;
    }
    /// формирование FROM секции запроса
    str += " FROM ";
    /// добавление список таблиц в FROM участок запроса
    if(subtable.size() == 0) str += tablename;
    else{
        QStringList::iterator itr;
        for(itr = subtable.begin(); itr != subtable.end(); ++itr)
        {
            if(itr != subtable.begin()) str += ", ";
            str += *itr;
        }
    }
    /// создание условия выборки - секции WHERE
    QMap<QString,int> numberofentries;                                          /// число вхождений каждой из таблиц - определяется кол-вом ключей принадлеащих каждой таблице
    if(subtable.size() > 0)
    {
        QStringList temp;                                                       /// для хранения найденной связки таблиц
        QStringList tmplist;                                                    /// для хранения результата

        QStringList::iterator itr;
        for(itr = subtable.begin(); itr != subtable.end(); ++itr)               /// взять таблицу
        {
            QStringList::iterator itr1;
            for(itr1 = subtable.begin(); itr1 != subtable.end(); ++itr1)        /// взять вторую таблицу из того же списка
            {
                if(itr != itr1 && !temp.contains(*itr1+*itr))                   /// 1 и 2 взятые таблицы не одна и та же
                {
                    /// получить ключ связывающий их (если таковы есть)
                    MyDataOfKey key = metadata->getKeyData(*itr,*itr1);
                    if(metadata->isError()) return false;
                    if(key.foreignkey.isEmpty() || key.primarykey.isEmpty()) break;
                    else{
                        QString str1 = *itr  + "." + key.foreignkey  + " = " +
                                      *itr1 + "." + key.primarykey + " ";
                        if(!numberofentries.contains(*itr))
                            numberofentries.insert(*itr,1);
                        else
                            numberofentries[*itr]++;
                        tmplist.append(str1);
                    }
                }
            }
        }
        if(tmplist.size() > 0)
        {
            str += " WHERE ";
            QStringList::iterator itr;
            for(itr = tmplist.begin(); itr != tmplist.end(); ++itr)
            {
                if(itr != tmplist.begin()) str += " AND ";
                    str += *itr + " ";

            }
        }
    }
    /// формирование участка ORDER BY
    QString field = "";
    if(numberofentries.size() > 0)
    {
        int      max = -1;
        QString  table;
        QStringList::iterator itr;
        for(itr = subtable.begin(); itr != subtable.end(); ++itr){
            if(numberofentries[*itr] > max){
                max = numberofentries[*itr];
                table = *itr;
            }
        }
        field = metadata->getPrimaryKey(table);
        if(metadata->isError()) return false;
    }
    else{
        field = metadata->getPrimaryKey(tablename);
        if(metadata->isError()) return false;
    }
    if(field != "")
        str += " ORDER BY " + field;

    /// сохранить текст запроса в lastquerytext
    lastquerytext = str;
    return true;
}

void MyTable::generateUpdateQuery()
{
    /// для каждой таблицы свой запрос
    /// обойти все поля в editablerecord
    /// структура запроса для каждой таблицы
    QStringList::iterator itr;  /// итератор по полям
    QStringList::iterator itr1; /// итератор по таблицам
    itr1 = tablenamelist.begin();
    int index = 0;
    for(itr = fieldlist.begin(); itr < fieldlist.end(); ++itr)
    {
        if(flagseditable[index]) /// в список включаются только редактируемые поля
        {
            QString curtable;
            if((*itr1).size() > 0)
                curtable = *itr1;
            else
                curtable = tablename;
            if(!fieldsforquery.contains(curtable))
            {
                fieldsforquery.insert(curtable,QStringList());
            }
            fieldsforquery[curtable].append(*itr);
        }
        ++itr1;
        ++index;
    }
}


bool MyTable::prepareForeignKeysDataForTable(QString table)
{
    QVector<MyDataOfKey>keys;
    metadata->getForeignKeys(table, keys);                                      /// получить список внешних ключей
    if(metadata->isError()) return false;
    if(keys.size() > 0)
    {
        QVector<MyDataOfKey>::iterator itr1;
        for(itr1 = keys.begin(); itr1 != keys.end(); ++itr1)
        {
            if(itr1->primarykey.size() == 0 || itr1->referencedtable.size() == 0) {}
            else
            {
                int index = fieldlist.indexOf(itr1->foreignkey);
                if(index >= 0)
                {
                    /// сгенерировать SQL запрос для получения данных из ссылаемой таблицы
                    MyDataReference ref;
                    ref.sqltext = "SELECT * FROM " + itr1->referencedtable +
                                  " ORDER BY "+itr1->primarykey;
                    QStringList alternamesoftable;
                    metadata->getAlterNamesFieldOfTable(itr1->referencedtable,alternamesoftable);
                    if(metadata->isError()) return false;
                    ref.alternames = alternamesoftable;
                    references.insert(index, ref);
                }
            }
        }
    }
    return true;
}

bool MyTable::prepareForeignKeysData()
{
    QStringList::iterator itr;
    if(subtable.size() > 0)
    {
        for(itr = subtable.begin(); itr != subtable.end(); ++itr)               /// для каждой субтаблицы
        {
            prepareForeignKeysDataForTable(*itr);
        }
    }
    else
        prepareForeignKeysDataForTable(tablename);
    return true;
}

bool MyTable::preparePrimaryKeysData()
{
    /// для таблицы необходимо запросить значения первичных
    /// ключей каждой таблицы
    ///
    /// получить список первичных ключей всех таблиц
    if(subtable.size() > 0)
    {
        QStringList::iterator itrs;
        for(itrs = subtable.begin(); itrs != subtable.end(); ++itrs)
        {
            QString pk = metadata->getPrimaryKey(*itrs);
            if(metadata->isError()) return false;
            /// если ключ отсутствует в структуре таблицы, его нужно добавить и скрыть
            if(!fieldlist.contains(pk))
                appendField(pk,*itrs,false,false);
            primarykeys.append(pk);
        }
    }
    else
    {
        QString pk = metadata->getPrimaryKey(tablename);
        if(metadata->isError()) return false;
        /// если ключ отсутствует в структуре таблицы, его нужно добавить и скрыть
        if(!fieldlist.contains(pk))
        {
            appendField(pk,tablename,false,false);
            primarykeys.append(pk);
        }
    }

    return true;

}
