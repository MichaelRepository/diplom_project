#include "dialogeditrecord.h"
#include "ui_dialogeditrecord.h"

DialogEditRecord::DialogEditRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditRecord)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::Tool | Qt::WindowStaysOnTopHint);

    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setSelectionMode(QTreeView::SingleSelection);
    ui->treeView->setSelectionBehavior(QTreeView::SelectRows);
    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setEditTriggers(QAbstractItemView::CurrentChanged |
                                  QAbstractItemView::SelectedClicked);
}

DialogEditRecord::~DialogEditRecord()
{
    delete ui;
}

void DialogEditRecord::setModel(EditRecordModel *model, QList<QString> *regexplist)
{
    recordmodel = model;
    ui->treeView->setModel(recordmodel);
    ui->treeView->resizeColumnToContents(0);
    RecordDelegate* delegate = new RecordDelegate();
    delegate->setRegStrList(regexplist);
    ui->treeView->setItemDelegate(delegate);
}
