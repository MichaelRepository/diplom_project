#ifndef DIALOGEDITRECORD_H
#define DIALOGEDITRECORD_H

#include <QDialog>
#include <QDate>
#include <QSql>

#include "MyTable/myeditrecordmodel.h"
#include "recorddelegate.h"
#include "subtablewidget.h"

namespace Ui {
class DialogEditRecord;
}

class DialogEditRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditRecord(QString connectname, QWidget *parent = 0);
    ~DialogEditRecord();

    void setRecord(MySqlRecord*  record);                                       /// передать ссылку на таблицу
    void showEvent(QShowEvent *event);

private slots:
    void on_cancellbt_clicked();
    void on_confirmbt_clicked();

private:
    void closeEvent(QCloseEvent *evnt);
    Ui::DialogEditRecord *ui;
    MySqlRecord*  recorddata;
    EditRecordModel* recordmodel;
    QString connectionname;
    RecordDelegate* delegate;
};

#endif // DIALOGEDITRECORD_H
