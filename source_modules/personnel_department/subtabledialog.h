#ifndef SUBTABLEDIALOG_H
#define SUBTABLEDIALOG_H

#include <QDialog>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QInputEvent>
#include <QSql>

#include "dbmessdlg.h"
#include "QSpreadsheetHeaderView.h"

enum SubTable{CITIZENSHIP, RESIDENCE, PASSPORT, REPRESENT,
              EDUCATION, PRIVILEGES, /*PROGRESS,*/ MOREINF};

namespace Ui {
class SubTableDialog;
}

class SubTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubTableDialog(QString connectname, QWidget *parent = 0);
    ~SubTableDialog();

    void setTitleText(QString text);                                            /// установить заголовок
    void ExecSqlQuery(SubTable table,    int keyvalue);                         /// установить параметры (подтаблица,   значение ключа)     запроса (отобразить зависимую таблицу)
    void ExecSqlQuery(QString  sqlstring);                                      /// установить параметры (текст запрос) запроса (отобразить результат запроса)
    void setHeadersNameList(QStringList &namelist);                             /// установить список имен заголовка таблицы

    void setDisplayMode(bool titleVisible           = true,                     /// отображать заголовок
                        bool editbuttonpanelVisible = true,                     /// отображение панели с кнопками редактирования
                        bool dlgMovable             = true);                    /// перемещение окна


    QVariant getCurRecordAttributeValue(QString attribute);                     /// получить значение атрибута текущей записи

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent * event);

private slots:
    void on_tableView_clicked(const QModelIndex &index);

private:
    Ui::SubTableDialog *ui;
    QStatusBar*         statusbar;
    QPoint              mpos;
    bool                movable;                                                /// перемещение окна

    QSpreadsheetHeaderView* header;                                             /// заголовок таблицы (вьювера)
    dbMessDlg*              dbmess;                                             /// сообщение об ошибках СУБД
    QSqlQueryModel*         model;                                              /// модель данных из запроса
    QSqlQuery*              query;                                              /// текущий запрос

    SubTable                subtable;                                           /// текущая подтаблица
    int                     currentrow;                                         /// текущая строка(запись) таблицы
    QStringList             headernamelist;                                     /// список заголовков таблицы
};

#endif // SUBTABLEDIALOG_H
