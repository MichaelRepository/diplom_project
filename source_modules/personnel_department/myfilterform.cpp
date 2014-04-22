#include "myfilterform.h"
#include "ui_myfilterform.h"

MyFilterForm::MyFilterForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::MyFilterForm)
{
    ui->setupUi(this);
    tabledata = 0;
    ui->treeView->setItemDelegate(&filterdelegate);
    connect(ui->treeView->header(), &QHeaderView::sectionResized,
            this,                   &MyFilterForm::resizeViewColumn);
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

    ui->treeView->header()->setMinimumSectionSize(170);
    ui->treeView->header()->resizeSection(1,170);
    ui->treeView->header()->resizeSection(2,117);
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
    ui->treeView->resizeColumnToContents(0);
}

void MyFilterForm::resizeViewColumn(int col, int oldsize, int newsize)
{
    /*if(col == 0 && (newsize-oldsize) != 30){
        ui->treeView->setColumnWidth(0, newsize+30);
    }*/
}

