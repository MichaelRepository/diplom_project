#ifndef MYDOCUMENTODF_H
#define MYDOCUMENTODF_H

#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QRegExpValidator>
#include <QGridLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QLineEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QMovie>

#include <QDebug>
#include <QDialog>
#include <QObject>
#include <QtXml>
#include <QtSql>

#include "QuaZIP/quazip.h"
#include "QuaZIP/quazipfile.h"

/** ****************************************************************************
 * The MyDocumentODF class
 * данный класс читает исходный массив с шаблоном (готовым документом)
 * разбивает его на логические блоки
 * позволяет произвести замену элементов документа на заданные
 *
 ******************************************************************************/
enum MyDocErrors     {NOERROR,                                                  /// без ошибок
                      ZIPFILEERROR,                                             /// ошибка при чтении архива
                      XMLSTRUCTERROR,                                           /// ошибка в xml структуре
                      OBJECTSTRUCTERROR,                                        /// ошибка в структуре описания объекта подставновки
                      DATANOTFOUND                                              /// данные отсутствуют
                     };
enum MyRepObjectType {NOTYPE, TABLE, LIST, LINE};                               /// тип объекта подстановки
enum MyRepFieldType  {FIELD_NULL, STRING, NUM, DATE, ENUM};                     /// тип переменных объекта


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
    MyRepObjectData() {type = NOTYPE; valueisquery = false;}
    bool isValid(){ return (type != NOTYPE);}

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
        typevar = FIELD_NULL;
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

    void setConnectionName(QString connect);
    bool readDocumentData (QByteArray &data);                                   /// прочитать структуру файла
    bool saveDocumentCopy (QString newfile);                                    /// записать копию файла

    //QString lastError();
    bool isError() {return (error != NOERROR);}

private:
    bool readAllZippedFile(QByteArray &data);
    bool parsDocContent();                                                      /// получить структуру клюевых файлов
    void findVariableSet(const QDomElement &element,
                         QList<QDomElement> *variables);                        /// получить список полей-переменных
    MyRepObjectData parsVariableValue(QString value);                                      /// анализ струткуры значения поля-переменной
    bool replaceVariables(QDomElement &element, QDomDocument *dom);             /// замена поля-переменной на значение
    bool writeZippedFiles(QString file);                                        /// записать все файлы в новый массив

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
};

#endif // MYDOCUMENTODF_H
