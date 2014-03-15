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
    void ExecSqlQuery(SubTable table, int keyvalue);                            /// установить текст запроса

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::SubTableDialog *ui;
    QStatusBar*         statusbar;
    QPoint              mpos;

    QSpreadsheetHeaderView* header;                                             /// заголовок таблицы (вьювера)
    dbMessDlg*              dbmess;                                             /// сообщение об ошибках СУБД
    QSqlQueryModel*         model;                                              /// модель данных из запроса
    QSqlQuery*              query;                                              /// текущий запрос

    SubTable                subtable;                                           /// текущая подтаблица

};

#endif // SUBTABLEDIALOG_H
