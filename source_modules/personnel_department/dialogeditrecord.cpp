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
                                  QAbstractItemView::SelectedClicked|
                                  QAbstractItemView::DoubleClicked  |
                                  QAbstractItemView::EditKeyPressed);
}

DialogEditRecord::~DialogEditRecord()
{
    delete ui;
}

void DialogEditRecord::setTable(MyTable *_table)
{
    table = _table;
    ui->treeView->setModel(table->getCurrentRecordModel());
    /// создание и привязка делегата
    RecordDelegate* delegate = new RecordDelegate(this);
    delegate->setTable(table);
    ui->treeView->setItemDelegate(delegate);
    ui->treeView->resizeColumnToContents(0);
}

void DialogEditRecord::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    this->setFocus();
    this->activateWindow();
    this->raise();
}

void DialogEditRecord::on_cancellbt_clicked()
{
    done(-1);
}

void DialogEditRecord::on_confirmbt_clicked()
{
    done(1);
}

void DialogEditRecord::closeEvent(QCloseEvent *evnt)
{
    done(0);
}
