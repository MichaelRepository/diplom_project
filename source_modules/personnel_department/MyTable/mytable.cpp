#include "mytable.h"

MyTable::MyTable(MyInfoScheme *metadatascheme, const QString &connectname)
{
    init            = false;
    filtered        = false;
    currentrow      = -1;
    connectionname  = connectname;
    metadata        = metadatascheme;
    filter          = new MyFilterNode(0,"AND");
    /// привязка селекта и фильтра
    mainselect.setFilter(filter);
}

MyTable::~MyTable()
{
    delete filter;
}

void MyTable::appendField(const QString &field, const QString &table,
                          bool editable, bool visible)
{
    if(init)                                   return;
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
    mainselect.addSelectItem(table+'.'+field);                             /// добавление в структуру select запроса

    int subtableindex = tableIndexOf(table);
    if(subtableindex == -1)
    {
        subtables.append(MySubtable());                                         /// добавить новую субтаблицу, если такой еще небыло в списке
        subtableindex = subtables.size()-1;
        subtables[subtableindex].name = table;                                  /// задать имя субтаблице
        mainselect.addFromItem(table);                                     /// добавление таблицы в структуру select запроса
    }
    subtables[subtableindex].fields.append(pfield);                             /// добавить текущее поле в список полей субтаблицы
    pfield->table = &subtables[subtableindex];                                  /// передать полю указатель на его родительскую таблицу
    if(editable) ++subtables[subtableindex].editablefieldscount;                /// увеличить внутри субтабличный счетчик редактируемых записей
}

bool MyTable::appendLink(const QString &name,   const QString &key0_1,
                         const QString &key1_0, MyTable *table_1)
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

void MyTable::setFilterActivity(bool activ)
{
    filtered = activ & !filter->isEmpty();
}

void MyTable::setCurrentRow(int index){
    if(index > tablequery.size()) return ;
    currentrow = index;
}

int MyTable::getRecordsCount() const
{
    int rows = tablequery.size();
    if(rows == -1) return 0;
    else return rows;
}

int MyTable::displayedFieldsCount() const
{
    /// вернуть число видимых полей (столбцов)
    return (fieldforviewer.size() + links.size());
}

QVariant MyTable::getFieldValue(const QString &field)
{
    int index = fieldRealIndexOf(field);
    tablequery.seek(currentrow);
    return tablequery.value(index);
}

bool MyTable::updateData()
{
    if(!init) return false;
    tablequery.clear();
    QString querytext;
    if(!filtered && mainselect.customOrderIsEmpty())
        querytext = mainselecttext;
    else
        querytext = mainselect.selectQueryText(filtered);

    if(!tablequery.exec(querytext) )
    {
        sqlerror = tablequery.lastError();
        filtered = false;
        return false;
    }
    sqlerror = QSqlError();

    return true;
}

bool MyTable::setUpdateGroupOfRecords(const QList<int> &records)
{
    if(records.size() < 1) return false;
    QList<QPair<QString, QString> > fieldvalues;
    for(int i = 0; i < editablerecord.count(); ++i)
    {
        QString value = editablerecord.value(i).toString();
        QString field = editablerecord.fieldName(i);
        if(field.isEmpty() ) return false;
        if(!value.isEmpty()) fieldvalues.append(qMakePair(field, value) );
    }
    return setUpdateGroupOfRecords(records, fieldvalues);
}

bool MyTable::setUpdateGroupOfRecords(const QList<int> &records,
                                      const QList<QPair<QString, QString> > &field_value_list)
{
    if(!init || records.size() > tablequery.size() ) return false;

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()) return false;

    QStringList tables;                                                         /// список обновляемых таблиц
    QMap<QString, QStringList> valueslist;
    QMap<QString, QString>     primarylist;
    QMap<QString, QStringList> whereitems;                                      /// элементы участка where

    QList<QPair<QString, QString> >::const_iterator itr;
    for(itr = field_value_list.begin(); itr != field_value_list.end(); ++itr)
    {
        int fieldindex = fieldRealIndexOf((*itr).first);
        if(fieldindex == -1) return false;                                      /// отсутствует запрашиваемое поле
        MyField* field = &fields[fieldindex];
        if(!tables.contains(field->table->name) )
        {
            tables.append     (field->table->name);

            valueslist.insert (field->table->name, QStringList() );
            whereitems.insert (field->table->name, QStringList() );
            primarylist.insert(field->table->name, field->table->primarykey);
        }
        valueslist[field->table->name].append(field->table->name+"."+
                                              (*itr).first+" = "+(*itr).second);
    }

    QList<int>::const_iterator itrr;
    for(itrr = records.begin(); itrr != records.end(); itrr++)
    {
        if(!tablequery.seek((*itrr) ) ) return false;
        QStringList::const_iterator itrt;
        for(itrt = tables.begin(); itrt != tables.end(); ++itrt)
        {
            QStringList *wherelist = &whereitems[(*itrt) ];
            QString primary = primarylist[(*itrt)];
            QString pkvalue = tablequery.value(primary).toString();
            wherelist->append((*itrt)+"."+primary+" = "+"'"+pkvalue+"'");
        }
    }

    QSqlQuery query(db);
    db.transaction();
    QStringList::const_iterator itrt1;
    for(itrt1 = tables.begin(); itrt1 != tables.end(); ++itrt1)
    {
        QString querytext = "UPDATE "+(*itrt1)+" SET "+
                            valueslist[(*itrt1)].join(", ")+" WHERE "+
                            whereitems[(*itrt1)].join(" OR ");
        query.prepare(querytext);
        if(!query.exec())
        {
            sqlerror = query.lastError();
            db.rollback();
            return false;
        }
        sqlerror = QSqlError();
    }
    db.commit();
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
            sqlerror = query.lastError();
            db.rollback();
            return false;
        }
        sqlerror = QSqlError();
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
            sqlerror = QSqlError();
            QVariant lastid = query.lastInsertId();
            if(lastid.isValid())
                PKvalues.insert((*itrt1)->name,lastid);                         /// получить значение id объекта последней вставленной строки
        }

    }

    db.commit();
    return true;
}

bool MyTable::removeRecords(const QList<int> records)
{
    if(records.size() <= 0) return false;
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()){sqlerror = db.lastError(); return false;}

    db.transaction();
    QList<int>::const_iterator itrr;
    for(itrr = records.begin(); itrr!=records.end(); ++itrr)
    {
        if(!tablequery.seek((*itrr) ) )
        {
            db.rollback();
            return false;
        }

        QList<MySubtable>::const_iterator itr;
        for(itr = subtables.begin(); itr != subtables.end(); ++itr)
        {
            QVariant pkvalue = tablequery.value((*itr).primarykey );
            if(pkvalue.isValid() && !pkvalue.isNull())
            {
                QString querytex = "DELETE FROM "+(*itr).name+
                                   " WHERE "+(*itr).primarykey+" = "+
                                   "'"+pkvalue.toString()+"'";
                QSqlQuery query(db);
                if(!query.exec(querytex) )
                {
                    sqlerror = db.lastError();
                    db.rollback();
                    return false;
                }
                sqlerror = QSqlError();
            }
        }
    }
    db.commit();
    return true;
}

bool MyTable::removeRecord(int row)
{
    QList<int> records;
    records << row;
    return removeRecords(records);
}

bool MyTable::isFiltered() const
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

int MyTable::getCurrentRowIndex() const
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
        if((*itr).isEditable) editablerecord.append(&(*itr), record.value((*itr).name) );
    }

    return &editablerecord;
}

MySqlRecord *MyTable::getEmptyRecordForInsert()
{
    /// сформировать данные для копии запрашиваемой строки
    editablerecord.clear();
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).isEditable) editablerecord.append(&(*itr));
    }

    return &editablerecord;
}

QSqlError MyTable::lastSqlError() const
{
    return sqlerror;
}

MyFilterNode *MyTable::getFilterRoot() const
{
    return filter;
}

QList<const MyField * > MyTable::getFields() const
{
    QList<const MyField * > result;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        result.append(&(*itr));
    }
    return result;
}

QList<const MyField * > MyTable::getDisplayedFields() const
{
    QList<const MyField * > result;
    QList<MyField*>::const_iterator itr;
    for(itr = fieldforviewer.begin(); itr != fieldforviewer.end(); ++itr)
    {
        result.append(*itr);
    }
    return result;
}

QList<const MyField *> MyTable::getDisplayedLinkedTableFields() const
{
    QList<const MyField * > result;
    QList<MyLink>::const_iterator litr;
    for(litr = links.begin(); litr != links.end(); ++litr)
    {
        QList<const MyField * > linkedfields = (*litr).table_1->getFields();
        result.append(linkedfields);
    }
    return result;
}

QList<const MyField *> MyTable::getJoinedFieldsList() const
{
    QList<const MyField * > result;
    result.append(getFields());
    result.append(getDisplayedLinkedTableFields());
    return result;
}

QMap<int, QString> MyTable::getCustomOrdersData() const
{
    if(!init ) return QMap<int, QString> ();
    return mainselect.getCustomOrdersData();
}

void MyTable::setCustomOrderData(int column, QString data)
{
    if(!init ) return ;
    mainselect.setCustomOrderData(column, data);
}

void MyTable::clearCustomOrdersData()
{
    if(!init ) return ;
    mainselect.clearCustomOrdersData();
}

bool MyTable::contains(const QString &field) const
{
    if(fields.size() == 0) return false;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).name.toUpper() == field.toUpper()) return true;
    }
    return false;
}

void MyTable::createFilterForRecords(const QList<int> &records)
{
    filter->clear();
    if(!init || records.size() <= 0) return;
    QList<int>::const_iterator itr;

    QString pkfield = mainselect.getOrderItem(0);
    int index       = fieldRealIndexOf(pkfield);
    if(pkfield.isEmpty() || index == -1 ) return ;
    MyField* field  = &fields[index];

    for(itr = records.begin(); itr != records.end(); ++itr)
    {
        if(tablequery.seek((*itr) ) )
        {
            QVariant pkvalue = tablequery.value(pkfield);
            if(pkvalue.isValid() && !pkvalue.isNull() )
            {
                MyFilterNode* ornode = new MyFilterNode(0,"OR");
                MyFilterNode* node   = new MyFilterNode(0, field->table->name, pkfield,
                                                        "=", "'"+pkvalue.toString()+"'");
                ornode->addChild(node);
                filter->addChild(ornode);
            }
        }
    }
}

void MyTable::createQuickFilter(const QString& field, const QString& f_operator,
                                const QString& value)
{
    int index = fieldRealIndexOf(field);
    if(index == -1) return;
    MyField* curfield = &fields[index];
    MyFilterNode* node = new MyFilterNode(0,curfield->table->name, curfield->name,
                                          f_operator, value);
    filter->clear();
    filter->addChild(node);
}

int MyTable::fieldRealIndexOf(QString field) const
{
    if(fields.size() == 0) return -1;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).name.toUpper() == field.toUpper()) return (*itr).realindex;
    }
    return -1;
}

int MyTable::fieldViewIndexOf(QString field) const
{
    if(fields.size() == 0) return -1;
    QList<MyField>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        if((*itr).name.toUpper() == field.toUpper()) return (*itr).indexforviewer;
    }
    return -1;
}

int MyTable::tableIndexOf(QString table) const
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
            // получить реальный тип данных!
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
                if(!itrfk->referencedtable.isNull() )                           /// если referencedtable пуст, то поле-первичный ключ, иначе внешний
                {
                    /// сгенерировать SQL запрос для получения данных из ссылаемой таблицы
                    MyDataReference ref;
                    ref.sqltext = "SELECT * FROM " + (*itrfk).referencedtable +
                                  " ORDER BY "+(*itrfk).primarykey;
                    QStringList alternamesoftable;
                    metadata->getAlterNamesFieldOfTable((*itrfk).referencedtable,
                                                        alternamesoftable);
                    ref.alternames = alternamesoftable;
                    fields[index].reference = ref;

                    fields[index].isForeign = true;                             /// установить признак - внешний ключ
                }
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

QString MyTable::displayFieldAlterName(int index) const
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return QString();
    if(displayedFieldIsLink(index))
        return links[index-fieldforviewer.size()].name;
    else
        return fieldforviewer[index]->altername;
}

QString MyTable::displayFieldName(int index) const
{
    if(index < 0 || index >= (links.size()+fieldforviewer.size()) ) return QString();
    if(displayedFieldIsLink(index))
        return links[index-fieldforviewer.size()].name;
    else
        return fieldforviewer[index]->name;
}

QVariant MyTable::displayCellValue(int row, int col)
{
    if(!tablequery.seek(row) || col < 0 || col >= fieldforviewer.size() )
        return QVariant();
    return tablequery.value(fieldforviewer[col]->realindex);
}

QVariant MyTable::displayFieldValue(int row, const QString &field)
{
    int col = fieldViewIndexOf(field);
    return displayCellValue(row, col);
}

bool MyTable::displayedFieldIsForeign(int index) const
{
    if(index < 0 || index >= fieldforviewer.size() ) return false;
    return fieldforviewer[index]->isForeign;
}

bool MyTable::displayedFieldIsPrimary(int index) const
{
    if(index < 0 || index >= fieldforviewer.size() ) return false;
    return fieldforviewer[index]->isPrimary;
}

bool MyTable::displayedFieldIsLink(int index) const
{
    int realindex = index-fieldforviewer.size();
    return (realindex >= 0 && realindex < links.size());
}

MyLink *MyTable::getdisplayedLink(int index)
{
    int realindex = index-fieldforviewer.size();
    if(realindex < 0 || realindex >= links.size()) return 0;
    return &links[realindex];
}

void MyTable::generateSelectQuery()
{
    /// создание условия выборки - секции WHERE и ORDER
    QString fieldfororder;                                                      /// поле по которому производится группировка
    if(subtables.size() > 1)
    {
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
                    MyFilterNode* node = new MyFilterNode();
                    node->setTable((*itrfk).table);
                    node->setField((*itrfk).foreignkey);
                    node->setOperator("=");
                    node->setValue((*itrfk).referencedtable + "." +
                                   (*itrfk).primarykey);
                    mainselect.addWehereNode(node);
                }
            }
        }
    }
    else fieldfororder = subtables[0].primarykey;
    /// формирование участка ORDER BY
    mainselect.addOrderItem(fieldfororder);
    mainselecttext = mainselect.selectQueryText();
    //qDebug() << mainselecttext << "\n";
}
