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

class MyTable{};

class MyLinkedTables{                                                           /// класс для описания правил привязки таблиц

    void setTableObject     (MyTable* table)                                    /// установить указательна объект таблицы
    {

    }

    void setLinkedTableName (QString name)                                      /// установить имя таблицы
    {

    }
    void appendForeignKey   (QString primaryk, QString foreignk)                /// добавить внешний ключ
    {

    }
    void appendDataLink     (QString _fieldsource, QString _fieldrecipient)    /// добавить связь данных между тек. и линк. таблицами
    {

    }

    void getForeignKeysCount()                                                  /// получить число внешних ключей
    {

    }
    void getDataLinkCount()                                                     /// получить число связей данных (число связанных полей)
    {

    }
private:
    MyTable *tableobject;                                                       /// ссылка на объект таблицы
    QString tablename;                                                          /// имя слинкованной таблицы
    QStringList primarykeys;                                                    /// ее ключевые поля
    QStringList foreignkeys;                                                    /// соответсвующие ключевые поля данной таблицы
    QStringList fieldrecipient;                                                 /// поля-источники данных текущей таблицы
    QStringList fieldsource;                                                    /// поля-приёмники данных линкованной таблицы
};

class MyTable{
public:
    MyTable();
    void setFieldDisplayedInEditor(QString field);
    void setConnectionName        (QString name);
    void setAlterFieldsName       (const QStringList &list);
    void setFieldValidatorsData   (const QMap<QString, QString> &data);
    void setNoEditableFieldsList  (const QStringList &list);
    void setForeignKeys           (const QMap<QString,QPair<MyTable*,QString> > &fkeys);
    void setEditorKeyFields       (const QStringList &list);

    void setSelect (QString str);
    void setWhere  (QString str);
    void setFilter (QString str);
    void setOrderBy(QString str);
    void bindValues(QVariantList &values);

    bool     isForeignKey               (int field);
    bool     isForeignKey               (QString field);
    void     setCurrentRow              (int index);
    QVariant getCurrentRecordFieldValue (int index);
    QVariant getCurrentRecordFieldValue (QString field);
    QVariant getFieldAlterName          (int field);
    MyTable* getForeignTable            (QString foreignkey);
    MyTable* getForeignTable            (int foreignkey);
    QString  getFieldValidatorData      (int field);
    QString  getFieldValidatorData      (QString field);
    int      getRecordsCount();
    QVariant getCurrentRecordFieldDisplayedInEditorValue();

    void updateData();

    void execQuery   (bool select, bool where, bool filter, bool orderby);
    void applyEditing();
    void displayTable(QTableView* viewer);
    EditRecordModel *getCurrentRecordModel();

    QString     tablename;                                                      /// реальное имя таблицы!!!
private:
    QString     fielddisplayedineditor;                                         /// поле, значение которого будет отображено в редакторе
    QString     connectionname;

    QString     lastquerytext;                                                  /// хранит текст последнего SELECT запроса для обновления данных таблицы
    QString     selectsection;                                                  /// select участок запроса
    QString     wheresection;                                                   /// where  участок запроса
    QString     filtersection;                                                  /// where - filter участок
    QString     orderbysection;                                                 /// order by участок

    QString     updatequery;                                                    /// запрос на обновление данных
    QString     insertquery;                                                    /// запрос на добавление данных

    QStringList editorkeyfields;                                                /// ключевые поля, значения которых определяют обновляемые записи таблицы


    QStringList editoralternames;                                               /// альтернативное название заголовков для редактора
    QSqlRecord  editordatasource;                                               /// источник данных, участвующих в запросе на обновление/добавление
    EditRecordModel recordmodel;                                                /// модель данных записи (работает с копией записи таблицы)


    QStringList  alterhederlist;                                                /// заголовки таблиц
    QVariantList bindvalues;                                                    /// связываемые значения
    QMap<QString, QString> validatorsdata;                                      /// список рег. выр. для валидаторов полей
    QStringList  noeditablefields;                                              /// список не редактируемых полей (полей не отображаемых редактором)

    QMap<QString,QPair<MyTable*,QString> > foreignkeys;                         /// внешние ключи

    int currentrow;                                                             /// текущая строка таблицы

    QSqlQueryModel  tablemodel;                                                 /// модель данных таблицы
};
#endif // MYTABLE_H
