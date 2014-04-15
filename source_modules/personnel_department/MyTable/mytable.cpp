#include "mytable.h"

MyTable::MyTable(MyInfoScheme *metadatascheme, QString connectname)
{
    init       = false;
    currentrow = -1;
    filtered   = false;
    freeFilter = "";
    connectionname = connectname;
    metadata = metadatascheme;
}

void MyTable::appendField(QString field, QString table, bool editable, bool visible)
{
    if(field.size() == 0 || table.size() == 0) return;
    if(fieldRealIndexOf(field) != -1)          return;
    int fieldindex = fields.size();

    MyField newField;
    newField.name       = field;
    newField.isVisible  = visible;
    newField.isEditable = editable;
    newField.realindex  = fieldindex;
    if(visible) newField.indexforviewer = fieldforviewer.size();
    fields.append(newField);
    MyField* pfield = &fields[fieldindex];
    if(editable) fieldforeditor.append(pfield);
    if(visible)  fieldforviewer.append(pfield);

    int subtableindex = tableIndexOf(table);
    if(subtableindex == -1)
    {
        subtables.append(MySubtable());                                         /// добавить новую субтаблицу, если такой еще небыло в списке
        subtableindex = subtables.size()-1;
        subtables[subtableindex].name = table;                                  /// задать имя субтаблице
    }
    subtables[subtableindex].fields.append(pfield);                             /// добавить текущее поле в список полей субтаблицы
    pfield->table = &subtables[subtableindex];                                  /// передать полю указатель на его родительскую таблицу
    if(editable) ++subtables[subtableindex].editablefieldscount;                /// увеличить внутри субтабличный счетчик редактируемых записей
}

bool MyTable::appendLink(QString name,   QString key0_1,
                         QString key1_0, MyTable *table_1)
{
    if(!this   ->contains(key0_1)) return false;
    if(!table_1->contains(key1_0)) return false;

    MyLink newlink;
    newlink.name    = name;
    newlink.key0_1  = key0_1;
    newlink.key1_0  = key1_0;
    newlink.table_1 = table_1;

    links.append(newlink);
    return true;
}

bool MyTable::initializeTable()
{
    if(init)                  return false;
    if(metadata->isError())   return false;                                     /// проверка источника метаданных
    if(!checkSubtables())     return false;                                     /// проверка валидности субтаблиц
    if(fields.size() == 0)    return false;                                     /// проверка полей

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()) { sqlerror = db.lastError(); return false;}
    tablequery = QSqlQuery(db);

    if(!prepareSubtables())         return false;                               /// инициализировать данные субтаблиц
    if(!getExtendedDataOfFields())  return false;

    /// сформировать SELECT запрос
    generateSelectQuery();

    init = true;
    if(!updateData()) return false;

    return true;
}

void MyTable::setFilterForField(QString field, QString filter)
{
    int index = fieldRealIndexOf(field);
    if(index != -1) fields[index].filter = filter;
}

void MyTable::setFilterForTable(QString str)
{
    freeFilter = str;
}

void MyTable::setFilterActivity(bool activ)
{
    if(activ)
    {
        activ = false;
        QList<MyField>::const_iterator itr;
        for(itr = fields.begin(); itr != fields.end(); ++itr)
        {
            activ |= !(*itr).filter.isEmpty();
        }
        activ |= !freeFilter.isEmpty();
    }
    filtered = activ;
}

void MyTable::setCurrentRow(int index){
    currentrow = index;
}

int MyTable::getRecordsCount()
{
    return tablequery.size();
}

int MyTable::displayedFieldsCount()
{
    /// вернуть число видимых полей (столбцов)
    return (fieldforviewer.size() + links.count());
}

QVariant MyTable::getFieldValue(QString field)
{
    tablequery.seek(currentrow);
    return tablequery.value(field);
}

bool MyTable::updateData()
{
    if(!init)                  return false;
/// основной запрос
    QString querytext = lastselecttext;
    if(filtered)
    {
        if(freeFilter.contains("WHERE",Qt::CaseInsensitive))
            freeFilter.remove("WHERE",Qt::CaseInsensitive);

        QString filtersection;
        if(!querytext.contains("WHERE"))
            filtersection += " WHERE ";
        else
            filtersection += " AND ";

       QList<MyField>::const_iterator itr;
        int i = 0;
        int filteredfieldcount = 0;
        for(itr = fields.begin(); itr != fields.end(); ++itr)
        {
            if((*itr).filter.size() > 0)
            {
                if(filteredfieldcount > 0) filtersection += " AND ";
                filtersection += (*itr).name+" = '"+(*itr).filter+"'";
                ++filteredfieldcount;
            }
            ++i;
        }

        if(freeFilter.size() > 0)
        {
            if(filteredfieldcount > 0) filtersection += " AND ";
            filtersection += freeFilter;
        }

        if(lastselecttext.contains("ORDER BY",Qt::CaseInsensitive))
        {
            int pos = lastselecttext.indexOf("ORDER BY",Qt::CaseInsensitive);
            querytext.insert(pos, filtersection+" ");
        }
        else
            querytext += filtersection;
    }

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
    if(!tablequery.seek(currentrow) ) return false;
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()) return false;

    QStringList querylist;
    QList<MySubtable>::const_iterator itrt;
    for(itrt = subtables.begin(); itrt != subtables.end(); ++itrt)
    {
        if((*itrt).editablefieldscount > 0)
        {
            int count = 0;
            QString querytext = "UPDATE "+(*itrt).name+" SET ";
            const QList<MyField*> fieldlist = (*itrt).fields;
            QList<MyField*>::const_iterator itrlf;
            for(itrlf = fieldlist.begin(); itrlf != fieldlist.end(); ++itrlf)
            {
                if((*itrlf)->isEditable)
                {
                    if(count > 0) querytext += " , ";
                    querytext += (*itrlf)->name+" = '"+editablerecord.value((*itrlf)->name).toString()+"' ";
                    ++count;
                }
            }
            querytext += " WHERE "+(*itrt).primarykey+" = '"+tablequery.value((*itrt).primarykey).toString()+"' ";
            querylist.append(querytext);
        }
    }
    QSqlQuery query(db);
    db.transaction();
    QStringList::const_iterator itrl;
    for(itrl = querylist.begin(); itrl != querylist.end(); ++itrl)
    {
        query.prepare(*itrl);
        if(!query.exec())
        {
            db.rollback();
            return false;
        }
    }
    db.commit();/// окончить транзакцию
    return true;
}

bool MyTable::setInsertDataFields()
{
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()) return false;
    QSqlQuery query(db);

    QList<MySubtable*> insertionorder;/// список таблиц отсортированных по числу FK
    QList<MySubtable>::iterator itrt;
    /// перенести список таблиц
    for(itrt = subtables.begin(); itrt != subtables.end(); ++itrt)
    {
        insertionorder.append(&(*itrt));
    }

    /// сортировать
    QList<MySubtable*>::iterator itrt1;
    QList<MySubtable*>::iterator itrt2;
    for(itrt1 = insertionorder.begin(); itrt1 != insertionorder.end(); ++itrt1)
    {
        /// (10 - константа, ничего не значащая, главное, что она ВСЕГДА больше, чем реальное число FK в любой субтаблице из списка)
        int mincountfk = 10;
        QList<MySubtable*>::iterator itrt3 = insertionorder.end();
        for(itrt2 = itrt1; itrt2 != insertionorder.end(); ++itrt2)
        {
            if(mincountfk > (*itrt2)->foreignkeys.size())
            {
                mincountfk = (*itrt2)->foreignkeys.size();
                itrt3 = itrt2;
            }
        }
        if(itrt3 != insertionorder.end())
        {
            MySubtable* table = *itrt1;
            *itrt1 = *itrt3;
            *itrt3 = table;
        }
    }

    db.transaction();
    /// сформировать insert запрос
    QMap<QString, QVariant> PKvalues;
    for(itrt1 = insertionorder.begin(); itrt1 != insertionorder.end(); ++itrt1)
    {
        QString querytext = "INSERT INTO "+(*itrt1)->name;
        QStringList fieldssection;
        QStringList valuessection;

        const QList<MyField*>* tablefields = &((*itrt1)->fields);

        QList<MyField*>::const_iterator itrf;
        for(itrf = tablefields->begin(); itrf != tablefields->end(); ++itrf)
        {
            if((*itrf)->isEditable)
            {
                QVariant value;
                value = editablerecord.value((*itrf)->name);
                if(value.isValid() && value.toString().size() > 0)
                {
                    fieldssection.append( (*itrf)->name );
                    valuessection.append( "'" + value.toString() + "' " );
                }
            }
        }

        /// проверка внешних ключевых полей
        const QList<MyDataOfKey> *keys = &(*itrt1)->foreignkeys;
        QList<MyDataOfKey>::const_iterator itrfk;
        for(itrfk = keys->begin(); itrfk != keys->end(); ++itrfk)
        {
            QVariant value;
            if(!fieldssection.contains((*itrfk).foreignkey) )                         /// ключевое поле не использовалось, нужно его использовать
            {
                value = PKvalues[(*itrfk).referencedtable];
                if(value.isValid() && value.toString().size() > 0)
                {
                    fieldssection.append( (*itrfk).foreignkey );
                    valuessection.append( "'" + value.toString() + "' " );
                }
                break;
            }
        }

        if(fieldssection.size() > 0 && valuessection.size() > 0)
        {
            querytext += " (" + fieldssection.join(" , ") + ") VALUES (" + valuessection.join(" , ") + ") ";

            if(!query.exec(querytext))
            {
                sqlerror = query.lastError();
                db.rollback();
                return false;
            }
            QVariant lastid = query.lastInsertId();
            if(lastid.isValid())
                PKvalues.insert((*itrt1)->name,lastid);                             /// получить значение id объекта последней вставленной строки
        }

    }

    db.commit();
    return true;
}

bool MyTable::removeRecord(int row)
{
    if(!tablequery.seek(row)){sqlerror = tablequery.lastError();  return false;}
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()){sqlerror = db.lastError(); return false;}

    db.transaction();
    QList<MySubtable>::const_iterator itr;
    int index = 0;
    for(itr = subtables.begin(); itr != subtables.end(); ++itr)
    {
        QString querytex = "DELETE FROM "+(*itr).name+" WHERE "+(*itr).primarykey+" = ";
        querytex += "'"+tablequery.value((*itr).primarykey).toString()+"'";
        QSqlQuery query(db);
        if(!query.exec(querytex) )
        {
            sqlerror = db.lastError();
            db.rollback();
            return false;
        }
        ++index;
    }
    db.commit();
    return true;
}

bool MyTable::isFiltered()
{
    return filtered;
}

QList<int> MyTable::searchRecordByField(QString field, QString value)
{
    QList<int> list;
    int index = fieldRealIndexOf(field);
    if(index == -1 || !tablequery.first() ) return list;
    int row = 0;
    do
    {
        QString curvalue  = tablequery.value(index).toString();
        if(curvalue.contains(value, Qt::CaseInsensitive)) list.append(row);
        ++row;
    }
    while(tablequery.next());

    return list;
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
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).isEditable)
        {
            MySqlField field(record.field((*itr).name) );
            field.setAlterName((*itr).altername);
            field.setValidator((*itr).validator);
            field.setEditable ((*itr).isEditable);
            field.setVisible  ((*itr).isVisible);
            if((*itr).reference.sqltext.size() > 0)
            {
                field.setReference((*itr).reference);
                field.setAlterField((*itr).alterfield);
            }
            editablerecord.append(field);
        }
    }

    return &editablerecord;
}

MySqlRecord *MyTable::getEmptyRecordForInsert()
{
    /// сформировать данные для копии запрашиваемой строки
    editablerecord.clear();
    if(!tablequery.isSelect())
    {
        if(!tablequery.seek(0)) return &editablerecord;
    }

    QSqlRecord record = tablequery.record();
    record.clearValues();
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).isEditable)
        {
            MySqlField field(record.field((*itr).name) );
            field.setAlterName((*itr).altername);
            field.setValidator((*itr).validator);
            field.setEditable ((*itr).isEditable);
            field.setVisible  ((*itr).isVisible);
            if((*itr).reference.sqltext.size() > 0)
            {
                field.setReference((*itr).reference);
                field.setAlterField((*itr).alterfield);
            }
            editablerecord.append(field);
        }
    }

    return &editablerecord;

}

QSqlError MyTable::lastSqlError()
{
    return sqlerror;
}

bool MyTable::contains(QString field)
{
    if(fields.size() == 0) return false;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).name.toUpper() == field.toUpper()) return true;
    }
    return false;
}

int MyTable::fieldRealIndexOf(QString field)
{
    if(fields.size() == 0) return -1;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).name.toUpper() == field.toUpper()) return (*itr).realindex;
    }
    return -1;
}

int MyTable::fieldViewUndexOf(QString field)
{
    if(fields.size() == 0) return -1;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).name.toUpper() == field.toUpper()) return (*itr).indexforviewer;
    }
    return -1;
}

int MyTable::tableIndexOf(QString table)/// получить номер таблицы
{
    if(subtables.size() == 0) return -1;
    QList<MySubtable>::const_iterator itr;
    int index = 0;
    for(itr = subtables.begin(); itr != subtables.end(); ++itr)
    {
        if((*itr).name.toUpper() == table.toUpper()) return index;
        ++index;
    }
    return -1;
}

bool MyTable::checkSubtables() /// проверка валидности субтаблиц
{
    if(subtables.size() == 0) return false;
    QList<MySubtable>::const_iterator itr;
    for(itr = subtables.begin(); itr != subtables.end(); ++itr)
    {
        if(!metadata->tableIsValid((*itr).name)) return false;
    }
    return true;
}

bool MyTable::prepareSubtables()/// подготовить данные каждой субтаблицы
{
    if(subtables.size() == 0) return false;
    QList<MySubtable>::iterator itr;
    QList<MySubtable>::const_iterator itr1;
    for(itr = subtables.begin(); itr != subtables.end(); ++itr)
    {
        QString pk = metadata->getPrimaryKey((*itr).name);                      /// получить первичный ключ субтаблицы
        (*itr).primarykey = pk;
        /// если ключ отсутствует в структуре объекта MyTable, его нужно добавить и спрятать
        /// такое решение необходимо того, чтобы обладать доступом к
        /// служебным данным без оброщения к БД
        if(!contains(pk)) appendField(pk,(*itr).name,false,false);

        /// получение списка внешних ключей, связывающих текущую субтаблицу с другими субтаблицами
        for(itr1 = subtables.begin(); itr1 != subtables.end(); ++itr1)
        {
            if(itr1 != itr)
            {
                /// получить ключ связывающий субтаблицы (если таков есть)
                MyDataOfKey key = metadata->getKeyData((*itr).name,(*itr1).name);
                if(key.foreignkey.isEmpty() || key.primarykey.isEmpty()) break;
                (*itr).foreignkeys.append(key);
            }
        }
    }
    return true;
}

bool MyTable::getExtendedDataOfFields()
{
    QList<MySubtable>::iterator itrt;
    QList<MyField*>  ::iterator itrf;
    for(itrt = subtables.begin(); itrt != subtables.end(); ++itrt)
    {
        QList<MyField*> *tablefields = &(*itrt).fields;
        for(itrf = tablefields->begin(); itrf != tablefields->end(); ++itrf)
        {
            (*itrf)->altername  = metadata->getAlterName ((*itrt).name,(*itrf)->name);
            (*itrf)->validator  = metadata->getValidator ((*itrt).name,(*itrf)->name);
            (*itrf)->alterfield = metadata->getAlterField((*itrt).name,(*itrf)->name);
        }

    }
    /// обработка полей - внешних ключей
    for(itrt = subtables.begin(); itrt != subtables.end(); ++itrt)
    {
        QVector<MyDataOfKey> fklist;
        QVector<MyDataOfKey>::iterator itrfk;
        /// получить все внешние ключи таблицы
        metadata->getForeignKeys((*itrt).name, fklist);
        for(itrfk = fklist.begin(); itrfk != fklist.end(); ++itrfk)
        {
            int index = fieldRealIndexOf((*itrfk).foreignkey);
            /// если текущий ключ присутствует в списке полей
            if(index != -1)
            {
                /// сгенерировать SQL запрос для получения данных из ссылаемой таблицы
                MyDataReference ref;
                ref.sqltext = "SELECT * FROM " + (*itrfk).referencedtable +
                              " ORDER BY "+(*itrfk).primarykey;
                QStringList alternamesoftable;
                metadata->getAlterNamesFieldOfTable((*itrfk).referencedtable, alternamesoftable);
                ref.alternames = alternamesoftable;
                fields[index].reference = ref;
                if(!itrfk->referencedtable.isNull() )                           /// если referencedtable пуст, то поле-первичный ключ, иначе внешний
                    fields[index].isForeign = true;                             /// установить признак - внешний ключ
                else
                    fields[index].isPrimary = true;                             /// установить признак - первичный ключ
            }
        }
    }

    return !metadata->isError();
}

bool MyTable::prepareLinks()
{
   /* ??? */
    return true;
}

QString MyTable::displayFieldAlterName(int index)
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return QString();
    if(displayedFieldIsLink(index)) return links[index-fieldforviewer.size()].name;
    return fieldforviewer[index]->altername;
}

QString MyTable::displayFieldName(int index)
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return QString();
    if(displayedFieldIsLink(index)) return links[index-fieldforviewer.size()].name;
    return fieldforviewer[index]->name;
}

QVariant MyTable::displayCellValue(int row, int col)
{
    if(!tablequery.seek(row) || col < 0 ||
       col > (fieldforviewer.size()+links.size()-2) ) return QVariant();
    if(displayedFieldIsLink(col)) return QVariant();
    return tablequery.value(fieldforviewer[col]->realindex);
}

QVariant MyTable::displayFieldValue(int row, QString field)
{
    int col = fieldViewUndexOf(field);
    return displayCellValue(row, col);
}

bool MyTable::displayedFieldIsForeign(int index)
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return false;
    if(displayedFieldIsLink(index)) return false;
    return fieldforviewer[index]->isForeign;
}

bool MyTable::displayedFieldIsPrimary(int index)
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return false;
    if(displayedFieldIsLink(index)) return false;
    return fieldforviewer[index]->isPrimary;
}

bool MyTable::displayedFieldIsLink(int index)
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return false;
    return (index >= (fieldforviewer.size()));
}

MyLink * const MyTable::getdisplayedLink(int index)
{
    if(!displayedFieldIsLink(index)) return 0;
    return &links[index-fieldforviewer.size()];
}

void MyTable::generateSelectQuery()
{
    QString querytext = "SELECT ";

    /// перенос списка полей в запрос (в порядке добавления полей в таблицу!)
    QList<MyField>::const_iterator itrf;
    for(itrf = fields.begin(); itrf != fields.end(); ++itrf)
    {
        if(itrf != fields.begin())  querytext += " , ";
        querytext +=  (*itrf).table->name + "." + (*itrf).name;
    }

    /// формирование FROM секции запроса
    querytext += " FROM ";
    /// добавление списка таблиц в FROM участок запроса
    QList<MySubtable>::const_iterator itrt;
    for(itrt = subtables.begin(); itrt != subtables.end(); ++itrt)
    {
        if(itrt != subtables.begin()) querytext += " , ";
        querytext += (*itrt).name;
    }

    /// создание условия выборки - секции WHERE и ORDER
    QString fieldfororder;                                                      /// поле по которому производится группировка
    if(subtables.size() > 1)
    {
        int items = 0;                                                          /// число элементов, добавленных в WHERE
        int maxcountfk = -1;                                                    /// максимальное число fk
        QList<MySubtable>::const_iterator itrt1;
        for(itrt1 = subtables.begin(); itrt1 != subtables.end(); ++itrt1)
        {
            const QList<MyDataOfKey> *fkeys = &(*itrt1).foreignkeys;
            if(maxcountfk < fkeys->size())
            {
                maxcountfk = fkeys->size();
                fieldfororder = (*itrt1).primarykey;
            }
            if((*fkeys).size() > 0)
            {
                QList<MyDataOfKey>::const_iterator itrfk;
                for(itrfk = fkeys->begin(); itrfk != fkeys->end(); ++itrfk)
                {
                    if(items == 0 ) querytext += " WHERE ";
                    else querytext += " AND ";
                    querytext += (*itrfk).table           + "." + (*itrfk).foreignkey  + " = " +
                                 (*itrfk).referencedtable + "." + (*itrfk).primarykey  + " ";
                    ++items;
                }
            }
        }
    }
    else fieldfororder = subtables[0].primarykey;

    /// формирование участка ORDER BY
    querytext += " ORDER BY " + fieldfororder;
    lastselecttext = querytext;
}

/*bool MyTable::prepareForeignKeysDataForTable(QString table)
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
}*/

