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

void DialogEditRecord::setModel(EditRecordModel *model, QList<QString> *regexplist,
                                QMap<QString, SubTableWidget *> *tableattributelist)
{
    /// приявязка вьювера к модели
    recordmodel = model;
    ui->treeView->setModel(recordmodel);
    ui->treeView->resizeColumnToContents(0);
    /// создание и привязка делегата
    RecordDelegate* delegate = new RecordDelegate(this);
    delegate->setRegStrList(regexplist);                                        /// передача списка рег выражнией делегату
    delegate->setTableAttributeList(tableattributelist);                        /// передать список атрибутов, являющихся субтаблицами

    ui->treeView->setItemDelegate(delegate);
}
