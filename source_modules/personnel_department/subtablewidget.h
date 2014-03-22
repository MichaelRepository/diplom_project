#ifndef SUBTABLEWIDGET_H
#define SUBTABLEWIDGET_H

#include <QWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QInputEvent>
#include <QSql>

#include <QList>
#include <QPair>

#include "dbmessdlg.h"
#include "QSpreadsheetHeaderView.h"

enum SubTable{CITIZENSHIP, RESIDENCE, PASSPORT, REPRESENT,
              EDUCATION, PRIVILEGES, /*PROGRESS,*/ MOREINF};

namespace Ui {
class SubTableWidget;
}

class SubTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SubTableWidget(QString connectname = "",QWidget *parent = 0);
    ~SubTableWidget();

    void setTitleText(QString text);                                            /// установить заголовок
    void ExecSqlQuery(SubTable table,    int keyvalue);                         /// установить параметры (подтаблица,   значение ключа)     запроса (отобразить зависимую таблицу)
    void ExecSqlQuery(QString  sqlstring);                                      /// установить параметры (текст запрос) запроса (отобразить результат запроса)
    void setHeadersNameList(QStringList &namelist);                             /// установить список имен заголовка таблицы
    void setDisplayedField(QString field);                                      /// установить атрибут, значение которого отображается в редакторе
    bool setCurrentRow(int row);                                                /// установить номер текущей строки
    bool setCurrentRow(QList< QPair <QString, QVariant> > &searchoption);       /// поиск записи в запросе по значениям атрибутов и установка номера записи в качестве текущей

    void setDisplayMode(bool titleVisible           = true,                     /// отображать заголовок
                        bool editbuttonpanelVisible = true,                     /// отображение панели с кнопками редактирования
                        bool headerMenuVisible      = true,                     /// отображение контектсного меняю для заголовков субтаблицы
                        bool dlgMovable             = true,                     /// перемещение окна
                        Qt::FocusPolicy tablefocuspolicy = Qt::WheelFocus
                        );

    QVariant     getCurRecordAttributeValue(QString attribute);                 /// получить значение атрибута текущей записи
    QVariant     getDisplayedFieldValue();                                      /// получить значения атрибута DisplayedField

    int findRecordByAttributeList(QList<QPair<QString, QVariant> > &attrlist);  /// поиск записи по списку значений атрибутов

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void newrowselected();
private slots:
    void on_tableView_clicked(const QModelIndex &index);

private:
    void applyTableHeaders();                                                   /// применить ранее установленные заголовки
    void applyCurrentRow();                                                     /// спозиционироваться на указанную строку

    Ui::SubTableWidget *ui;
    QStatusBar*         statusbar;
    QPoint              mpos;
    bool                movable;                                                /// перемещение окна

    QString                 curconnectname;
    QSpreadsheetHeaderView* header;                                             /// заголовок таблицы (вьювера)
    dbMessDlg*              dbmess;                                             /// сообщение об ошибках СУБД
    QSqlQueryModel*         model;                                              /// модель данных из запроса

    SubTable                subtable;                                           /// текущая таблица
    int                     currentrow;                                         /// текущая строка(запись) таблицы
    QStringList             headernamelist;                                     /// список заголовков таблицы
    QString                 displayedfield;                                     /// имя атрибута значение которого будет отображено в редакторе
};

#endif // SUBTABLEWIDGET_H
