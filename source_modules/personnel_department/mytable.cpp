#include "mytable.h"


MyTable::MyTable(){
    currentrow = -1;
}

void MyTable::setConnectionName(QString name){
    connectionname = name;
}

void MyTable::setAlterFieldsName(QStringList &list){
    alterhederlist = list;
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

QVariant MyTable::getCurrentRecordFieldValue(QString field)
{
    if(currentrow == -1) return QVariant();
    return tablemodel.record(currentrow).value(field);
}

void MyTable::execQuery(bool select, bool where, bool filter, bool orderby)
{
    QString str = "";
    if(select)  str += selectsection;
    if(where)   str += wheresection;
    if(filter)  str += filtersection;
    if(orderby) str += orderbysection;

    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open()){
        dbMessDlg dbmess;
        dbmess.showdbmess(db.lastError());
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
    }
    QStringList::iterator itrf;
    int i = 0;
    for(itrf = alterhederlist.begin(); itrf != alterhederlist.end(); ++itrf){
        tablemodel.setHeaderData(i,Qt::Horizontal,QVariant(*itrf));
        ++i;
    }
}

void MyTable::displayTable(QTableView *viewer)
{
    viewer->setModel(&tablemodel);
    viewer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewer->resizeColumnsToContents();
    viewer->resizeRowsToContents();
}

QVariant MyTable::getCurrentRecordFieldValue(int index)
{
    if(currentrow == -1) return QVariant();
    return tablemodel.record(currentrow).value(index);
}
