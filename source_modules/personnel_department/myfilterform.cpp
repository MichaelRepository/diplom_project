#include "myfilterform.h"
#include "ui_myfilterform.h"

MyFilterForm::MyFilterForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::MyFilterForm)
{
    ui->setupUi(this);
    tabledata = 0;
    ui->treeView->setItemDelegate(&filterdelegate);
    QPushButton* bt1 = ui->buttonBox->button(QDialogButtonBox::Ok);
    QPushButton* bt2 = ui->buttonBox->button(QDialogButtonBox::Cancel);
    bt1->setText("Применить");
    bt2->setText("Отмена");
    connect(bt1,  &QPushButton ::clicked,
            this, &MyFilterForm::filterready);

    ui->addBt->setToolTip           ("Создать группу условий фильтрации");
    ui->addConditionBt->setToolTip  ("Создать условие фильтрации");
    ui->remBt->setToolTip           ("Удалить узел фильтра");
    ui->clearBt->setToolTip         ("Сбросить фильтр");

}

MyFilterForm::~MyFilterForm()
{
    delete ui;
}

void MyFilterForm::setTable(MyTable *table)
{
    tabledata = table;
    tablefilterroot = table->getFilterRoot();
    filtermodel.setFilterData(tablefilterroot, table->getJoinedFieldsList());
    ui->treeView->setModel(&filtermodel);

    ui->treeView->header()->setSectionsMovable(false);
    ui->treeView->header()->setVisible(true);
    ui->treeView->header()->setSectionsClickable(false);

    ui->treeView->header()->setMinimumSectionSize(100);
    ui->treeView->header()->resizeSection(1,170);
    ui->treeView->header()->resizeSection(2,90);
    ui->treeView->expandAll();
}

void MyFilterForm::on_addBt_clicked()
{
    QModelIndexList indexlist = ui->treeView->selectionModel()->selectedRows(0);
    if(indexlist.size() == 0)
        filtermodel.addConditionsGroup(QModelIndex());
    else
        filtermodel.addConditionsGroup(indexlist[0]);
    ui->treeView->expandAll();
    ui->treeView->resizeColumnToContents(0);
}

void MyFilterForm::on_addConditionBt_clicked()
{
    QModelIndexList indexlist = ui->treeView->selectionModel()->selectedRows(0);
    if(indexlist.size() == 0)
        filtermodel.addCondition(QModelIndex());
    else
        filtermodel.addCondition(indexlist[0]);
    ui->treeView->expandAll();
    ui->treeView->resizeColumnToContents(0);
}

void MyFilterForm::on_remBt_clicked()
{
    QModelIndexList indexlist = ui->treeView->selectionModel()->selectedRows(0);
    if(indexlist.size() == 0) return;
    filtermodel.removeChildItem(indexlist[0]);
    ui->treeView->expandAll();
    ui->treeView->resizeColumnToContents(0);
}

void MyFilterForm::on_clearBt_clicked()
{
    filtermodel.clearFilter();
    ui->treeView->expandAll();
    ui->treeView->repaint();
}

void MyFilterForm::on_buttonBox_rejected()
{
    close();
}

void MyFilterForm::on_ExpandBt_clicked()
{
    ui->treeView->expandAll();
}
