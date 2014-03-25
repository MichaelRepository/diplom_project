#include "mytable.h"


MyTable::MyTable(){
    currentrow = -1;
}

void MyTable::setFieldDisplayedInEditor(QString field)
{
    fielddisplayedineditor = field;
}

void MyTable::setConnectionName(QString name){
    connectionname = name;
}

void MyTable::setAlterFieldsName(const QStringList &list){
    alterhederlist = list;
}

void MyTable::setFieldValidatorsData(const QMap<QString, QString> &data)
{
    validatorsdata = data;
}

void MyTable::setNoEditableFieldsList(const QStringList &list)
{
    noeditablefields = list;
}

void MyTable::setForeignKeys(const QMap<QString, QPair<MyTable *, QString> > &fkeys)
{
    foreignkeys = fkeys;
}

void MyTable::setEditorKeyFields(const QStringList &list)
{
    editorkeyfields = list;
}

void MyTable::setSelect(QString str){
    selectsection  = str;
}

void MyTable::setWhere(QString str){
    wheresection   = str;
}

void MyTable::setFilter(QString str){
    filtersection  = str;
}

void MyTable::setOrderBy(QString str){
    orderbysection = str;
}

void MyTable::bindValues(QVariantList &values){
    bindvalues = values;
}

void MyTable::setCurrentRow(int index){
    currentrow = index;
}

int MyTable::getRecordsCount()
{
    return tablemodel.rowCount();
}

QVariant MyTable::getCurrentRecordFieldDisplayedInEditorValue()
{
    return getCurrentRecordFieldValue(fielddisplayedineditor);
}

void MyTable::updateData()
{
    if(lastquerytext == "") return ;

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()){
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
    }
    QSqlQuery query(db);
    query.prepare(lastquerytext);
    if(!query.exec() ){
        dbMessDlg dbmess;
        dbmess.showdbmess(query.lastError());
    }
    tablemodel.setQuery(query);
}

QVariant MyTable::getCurrentRecordFieldValue(QString field)
{
    if(currentrow == -1) return QVariant();
    return tablemodel.record(currentrow).value(field);
}

QVariant MyTable::getFieldAlterName(int field)
{
    if(field < alterhederlist.size())
        return alterhederlist.at(field);
    else return QVariant();
}

MyTable *MyTable::getForeignTable(QString foreignkey)
{
    if(foreignkeys.contains(foreignkey))
        return foreignkeys[foreignkey].first;
    else
        return 0;
}

MyTable *MyTable::getForeignTable(int foreignkey)
{
    QString keyfieldname = tablemodel.record(0).fieldName(foreignkey);
    if(foreignkeys.contains(keyfieldname))
        return foreignkeys[keyfieldname].first;
    else
        return 0;
}

QString MyTable::getFieldValidatorData(int field)
{
    QString fieldname = tablemodel.record(0).fieldName(field);
    if(validatorsdata.contains(fieldname))
        return validatorsdata[fieldname];
    else
        return "";
}

QString MyTable::getFieldValidatorData(QString field)
{
    if(validatorsdata.contains(field))
        return validatorsdata[field];
    else
        return "";
}

bool MyTable::isForeignKey(int field)
{
    QString fieldname = tablemodel.record(0).fieldName(field);
    return foreignkeys.contains(fieldname);
}

bool MyTable::isForeignKey(QString field)
{
    return foreignkeys.contains(field);
}

void MyTable::execQuery(bool select, bool where, bool filter, bool orderby)
{
    QString str = "";
    if(select)  str += "SELECT "+selectsection+" ";
    if(where)   str += "WHERE " +wheresection+" ";
    if(filter)  str += (where ? "AND " : "WHERE ")+filtersection +" ";
    if(orderby) str += "ORDER BY "+orderbysection;

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()){
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
        return ;
    }
    QSqlQuery query(db);
    query.prepare(str);
    if(where && bindvalues.size()>0)
    {
        QVariantList::iterator itrb;
        int i = 0;
        for(itrb = bindvalues.begin(); itrb != bindvalues.end(); ++itrb)
        {
            query.bindValue(i, *itrb);
            ++i;
        }
    }
    query.exec();
    tablemodel.setQuery(query);
    if(tablemodel.lastError().isValid()){
        dbMessDlg dbmess;
        dbmess.showdbmess(tablemodel.lastError());
        return ;
    }
    QStringList::iterator itrf;
    int i = 0;
    for(itrf = alterhederlist.begin(); itrf != alterhederlist.end(); ++itrf){
        tablemodel.setHeaderData(i,Qt::Horizontal,QVariant(*itrf));
        ++i;
    }
    lastquerytext = str; /// сохранить текст последнего запроса
}

void MyTable::applyEditing()
{
    QString updatesection;
    QTextStream querytext(&updatesection);
    /**
        формирование запроса на обновление данных:
        - анализируется объект editordatasource - запись, которая была
          отправлена на редактирование (отображалась в редакторе), значения
          которые были изменены пользователемвредакторе.
          В текст запроса добавляется название поля из editordatasource
          символ "=" и соответвующее полю значение.
        - затем анализируется editorkeyfields - список ключевых полей,
          по которым формируется условие запроса (секция WHERE),
          в текст запроса добавляется имя ключа из списка, символ "=", а затем
          из модели таблицы, выбирается текущая запись (та копию которой отправил
          в редактор), и из поля с именем ключа получается значение.
    **/
    querytext << "UPDATE " << tablename << " SET ";
    for(int i = 0; i < editordatasource.count(); ++i)
    {
        if(editordatasource.field(i).value().isValid())
        {
            QString str = editordatasource.field(i).value().toString();
            if(str != "" && str !=" ")
            {
                if(editordatasource.field(i).value().type() == QVariant::String)
                    str = "'"+str+"'";
                if( i != 0) querytext << " , ";
                querytext << editordatasource.fieldName(i) << " = "
                          << str << " ";
            }
        }
    }
    querytext << " WHERE ";
    QStringList::iterator itrk;
    for(itrk = editorkeyfields.begin(); itrk != editorkeyfields.end(); ++itrk)
    {
        if(itrk != editorkeyfields.begin()) querytext << ", ";
        querytext << *itrk << " = "
                  << tablemodel.record(currentrow).field(*itrk).value().toString();
    }
    qDebug() << updatesection;
    /// обработка транзакции
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()){
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
    }
    db.transaction();
    QSqlQuery query(db);
    query.prepare(updatesection);
    if(!query.exec())
    {
        dbMessDlg dbmess;
        dbmess.showdbmess(query.lastError());
        db.rollback();
    }
    else
        db.commit() ?  : db.rollback();
}

void MyTable::displayTable(QTableView *viewer)
{    
    viewer->setModel(&tablemodel);
    viewer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewer->resizeColumnsToContents();
    viewer->resizeRowsToContents();
}

EditRecordModel *MyTable::getCurrentRecordModel()
{

    /// отбор отображаемых в редакторе полей
    /// подготовка списка алтер. имен для отобранных полей
    editordatasource.clear();
    editoralternames.clear();
    for(int i = 0; i < tablemodel.record(currentrow).count(); ++i )
    {
        if(!noeditablefields.contains(tablemodel.record(currentrow).fieldName(i)) )
        {
            editordatasource.append(tablemodel.record(currentrow).field(i));
            editoralternames << alterhederlist[i];
        }
    }
    recordmodel.setData(&editordatasource);
    recordmodel.setAlterNames(&editoralternames);

    return &recordmodel;
}

QVariant MyTable::getCurrentRecordFieldValue(int index)
{
    if(currentrow == -1) return QVariant();
    return tablemodel.record(currentrow).value(index);
}
