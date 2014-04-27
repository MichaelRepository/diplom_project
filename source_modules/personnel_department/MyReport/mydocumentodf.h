#ifndef MYDOCUMENTODF_H
#define MYDOCUMENTODF_H

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QRegExpValidator>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QMovie>

#include <QDialog>
#include <QObject>
#include <QDebug>
#include <QtXml>
#include <QtSql>

#include <QFile>

#include "QuaZIP/quazipfile.h"
#include "QuaZIP/quazip.h"

/** ****************************************************************************
 * The MyDocumentODF class
 * данный класс читает исходный массив с шаблоном (готовым документом)
 * разбивает его на логические блоки
 * позволяет произвести замену элементов документа на заданные
 *
 ******************************************************************************/
enum MyDocErrors     {MYDOC_NOERROR,                                            /// без ошибок
                      MYDOC_ZIPFILEERROR,                                       /// ошибка при чтении архива
                      MYDOC_XMLSTRUCTERROR,                                     /// ошибка в xml структуре
                      MYDOC_OBJECTSTRUCTERROR,                                  /// ошибка в структуре описания объекта подставновки
                      MYDOC_DATANOTFOUND                                        /// данные отсутствуют
                     };
enum MyRepObjectType {MYREP_NOTYPE, MYREP_TABLE, MYREP_LIST, MYREP_LINE};                               /// тип объекта подстановки
enum MyRepFieldType  {MYREP_FIELD_NULL, MYREP_STRING, MYREP_NUM,
                      MYREP_DATE, MYREP_ENUM};                                  /// тип переменных объекта


class MyFileStructure{                                                          /// дом структура для content и styles
public:
    QDomDocument dom;
    /// навигаторы
    QDomElement documentroot;                                                   /// корень документа (office:document-styles/office:document-content)
    QDomElement automatic_styles;
    QDomElement master_styles;
    QDomElement master_page;
    QDomElement header;
    QDomElement footer;
    QDomElement body;
    QDomElement bodyroot;                                                       /// первый элемент тела документа
    /// поля-переменные
    QList<QDomElement> variables;
    QByteArray data;
};

class MyRepObjectData{                                                          /// данные объекта подстановки
public:
    MyRepObjectData() {type = MYREP_NOTYPE; valueisquery = false;}
    bool isValid(){ return (type != MYREP_NOTYPE);}

    MyRepObjectType type;                                                       /// тип объекта
    QString         value;                                                      /// значение объекта
    bool            valueisquery;                                               /// значение объекта - запрос/текст
};

class MyFileFromZip{                                                            /// для хранения данных по каждому составному файлу архива
public:
    QString     name;
    QByteArray  data;
};

class MyReplaceableField{                                                       /// класс замещаемого поля (переменные объекта подстановки)
public:
    MyReplaceableField() {
        pos = -1;
        leng = 0;
        typevar = MYREP_FIELD_NULL;
        value="";
    }
    int pos;                                                                    /// позиция в строке
    int leng;                                                                   /// число замещаемых символов в исходном тексте
    QString         namevar;                                                    /// имя переменной (поясняющий текст)
    QStringList     enumlist;                                                   /// для  enum - список вариантов
    MyRepFieldType  typevar;                                                    /// тип переменной (string num date enum)

    QString     value;                                                          /// значение на которое будет замещен объект
    QWidget     *widget;                                                        /// указатель на элемент с которого будет взято значение
};

class MyDocumentODF : public QObject
{
    Q_OBJECT
public:
    explicit MyDocumentODF(QObject *parent = 0);
    ~MyDocumentODF();

    void setConnectionName (QString connect);
    bool createDocumentFrom(QSqlQuery& querydata,
                            const QStringList &fieldsname,
                            const QList<int>& fields);    /// создать документ из данных запроса
    bool readDocumentData  (QByteArray &data);                                  /// прочитать структуру файла
    bool saveDocumentCopy  (QString newfile);                                   /// записать копию файла

    //QString lastError();
    bool isError() {return (error != MYDOC_NOERROR);}

private:
    bool readAllZippedFile(QByteArray &data);
    bool parsDocContent();                                                      /// получить структуру клюевых файлов
    void findVariableSet(const QDomElement &element, QList<QDomElement> *variables);                        /// получить список полей-переменных
    MyRepObjectData parsVariableValue(QString value);                                      /// анализ струткуры значения поля-переменной
    bool replaceVariables(QDomElement &element, QDomDocument *dom);             /// замена поля-переменной на значение
    bool writeZippedFiles(QString file);                                        /// записать все файлы в новый массив

    void createTable(MyFileStructure& forfile, QSqlQuery& querydata,
                     const QList<int>& fields, const QStringList &fieldsname);

    /// указатели на данные файлов из списка, для быстрого доступа
    MyFileFromZip* contentfile;
    MyFileFromZip* stylesfile;

    QString zipname;                                                            /// имя odt документа
    MyFileStructure      contentstruct;                                         /// структура content.xml
    MyFileStructure      stylesstruct;                                          /// структура styles.xml
    QList<MyFileFromZip> files;                                                 /// массивы данных для хранения всех файлов документа
    MyDocErrors error;

    QList<MyReplaceableField> globalrepfiellist;                                /// список замещаемых полей всех объектов подстановки в документе

    QString connection;                                                         /// имя соединения с БД
    QDialog *catdlg;
    QMovie  *catmovie;
    // данные для построения документа из данных текущей таблицы
    /*QSqlQuery  _querydata;
    QList<int> _fields;*/
};

#endif // MYDOCUMENTODF_H
