#ifndef MYTABLE_H
#define MYTABLE_H

#include <QTableView>
#include <QObject>
#include <QtSql>

#include "dbmessdlg.h"
#include "editrecordmodel.h"

enum Tables{SPECIALITY, GROUP, STUDENT};                                        /// список основных таблиц
enum extraTables{CITIZENSHIPLIST, PRIVILEGESCATEGORY, SCHOOL, TYPESCHOOL,       /// справочники - таблицы категорий
                 TYPEEDUCATION};

class MyTable{
public:
    MyTable();
    void setConnectionName (QString name);
    void setAlterFieldsName(QStringList &list);

    void setSelect (QString str);
    void setWhere  (QString str);
    void setFilter (QString str);
    void setOrderBy(QString str);
    void bindValues(QVariantList &values);

    void     setCurrentRow(int index);
    QVariant getCurrentRecordFieldValue(int index);
    QVariant getCurrentRecordFieldValue(QString field);

    void execQuery(bool select, bool where, bool filter, bool orderby);

    void displayTable(QTableView* viewer);
    EditRecordModel *getCurrentRecordModel();

private:
    QString     connectionname;

    QString     selectsection;                                                  /// select участок запроса
    QString     wheresection;                                                   /// where  участок запроса
    QString     filtersection;                                                  /// where - filter участок
    QString     orderbysection;                                                 /// order by участок

    QStringList  alterhederlist;                                                /// заголовки таблиц
    QVariantList bindvalues;                                                    /// связываемые значения

    int currentrow;                                                             /// текущая строка таблицы

    QSqlQueryModel  tablemodel;                                                 /// модель данных таблицы
    EditRecordModel recordmodel;                                                /// модель данных записи (работает с копией записи таблицы)
};
#endif // MYTABLE_H
