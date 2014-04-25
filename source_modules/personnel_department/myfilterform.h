#ifndef MYFILTERFORM_H
#define MYFILTERFORM_H

#include <QWidget>
#include <QStatusBar>
#include <QLayout>
#include <QPushButton>

#include <MyTable/mytable.h>
#include <MyTable/myfiltermodel.h>
#include <MyTable/mydelegateforitemfilter.h>

namespace Ui {
class MyFilterForm;
}

class MyFilterForm : public QWidget
{
    Q_OBJECT
public:
    explicit MyFilterForm(QWidget *parent = 0);
    ~MyFilterForm();

    void setTable(MyTable* table);

signals:
    void filterready();

private slots:
    void on_addBt_clicked();
    void on_addConditionBt_clicked();
    void on_remBt_clicked();
    void on_clearBt_clicked();
    void on_buttonBox_rejected();

    void on_ExpandBt_clicked();

private:
    Ui::MyFilterForm *ui;

    MyTable*                tabledata;
    MyFilterNode*           tablefilterroot;
    MyFilterModel           filtermodel;
    MyDelegateForItemFilter filterdelegate;
};

#endif // MYFILTERFORM_H
