#ifndef DIALOGEDITRECORD_H
#define DIALOGEDITRECORD_H

#include <QDialog>
#include <QDate>

#include "editrecordmodel.h"
#include "recorddelegate.h"
#include "subtabledialog.h"

namespace Ui {
class DialogEditRecord;
}

class DialogEditRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditRecord(QWidget *parent = 0);
    ~DialogEditRecord();


    void setModel(EditRecordModel* model,                                       /// установить модель данных
                  QList<QString>* regexplist = 0,                               /// передать список рег выражений валидаторам
                  QMap<QString, SubTableDialog*>* tableattributelist  = 0);     /// передать список атрибутов, являющихся субтаблицами

private:
    Ui::DialogEditRecord *ui;

    EditRecordModel* recordmodel;                                               /// модель данных для одной записи
};

#endif // DIALOGEDITRECORD_H
