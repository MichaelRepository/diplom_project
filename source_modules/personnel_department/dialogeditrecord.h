#ifndef DIALOGEDITRECORD_H
#define DIALOGEDITRECORD_H

#include <QDialog>
#include <QDate>

#include "editrecordmodel.h"
#include "recorddelegate.h"

namespace Ui {
class DialogEditRecord;
}

class DialogEditRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditRecord(QWidget *parent = 0);
    ~DialogEditRecord();

    void setModel(EditRecordModel* model, QList<QString>* regexplist);                                      /// установить модель данных

private:
    Ui::DialogEditRecord *ui;

    EditRecordModel* recordmodel;                                               /// модель данных для одной записи
};

#endif // DIALOGEDITRECORD_H
