#include "dialogeditrecord.h"
#include "ui_dialogeditrecord.h"

DialogEditRecord::DialogEditRecord(QString connectname, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditRecord)
{
    ui->setupUi(this);
    connectionname = connectname;
    recordmodel = new EditRecordModel(this);

    setWindowFlags(Qt::Dialog /*| Qt::Tool*/ | Qt::WindowStaysOnTopHint);

    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setSelectionMode(QTreeView::SingleSelection);
    ui->treeView->setSelectionBehavior(QTreeView::SelectRows);
    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setEditTriggers(QAbstractItemView::CurrentChanged |
                                  QAbstractItemView::SelectedClicked|
                                  QAbstractItemView::DoubleClicked  |
                                  QAbstractItemView::EditKeyPressed);
    delegate = 0;
}

DialogEditRecord::~DialogEditRecord()
{
    delete ui;
}

void DialogEditRecord::setRecord(MySqlRecord *record)
{
    recorddata = record;
    recordmodel->setRecordData(recorddata);
    ui->treeView -> setModel(recordmodel);
    /// очистить старые данные делегата
    if(delegate) delete delegate;
    /// создание и привязка нового делегата
    delegate = new RecordDelegate(connectionname, this);
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
