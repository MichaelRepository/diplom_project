#include "subtablewidget.h"
#include "ui_subtablewidget.h"

SubTableWidget::SubTableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubTableWidget)
{
    ui->setupUi(this);

    statusbar       = new QStatusBar(this);
    header          = new QSpreadsheetHeaderView(Qt::Horizontal,this);
    standartheader  = new QHeaderView(Qt::Horizontal,this);
    movable   = true;                                                           /// разрешить перемещение окна
    ui->tableView->setHorizontalHeader(header);

    QLayout* layoutWidget = ui->gridLayout_2;
    layoutWidget->setContentsMargins(0,0,0,0);
    layoutWidget->setSpacing(0);
    layoutWidget->addWidget(statusbar);
}

SubTableWidget::~SubTableWidget()
{
    delete ui;
}

void SubTableWidget::setQueryModel(QSqlQueryModel *model)
{
   querymodel = model;
   ui->tableView->setModel(querymodel);
   ui->tableView->resizeColumnsToContents();
   ui->tableView->resizeRowsToContents();
}

void SubTableWidget::setTitleText(QString text)
{
    ui->label->setText(text);
}

void SubTableWidget::setDisplayMode(bool titleVisible,
                                    bool editbuttonpanelVisible,
                                    bool headerMenuVisible,
                                    bool dlgMovable,
                                    Qt::FocusPolicy tablefocuspolicy)
{
    movable = dlgMovable;
    ui->label->setVisible(titleVisible);
    ui->toolsframe->setVisible(editbuttonpanelVisible);
    ui->tableView->setFocusPolicy(tablefocuspolicy);
    if(!headerMenuVisible)
    {
        ui->tableView->setHorizontalHeader(standartheader);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->resizeRowsToContents();
    }
    QString style = "QAbstractItemView:item::selected{color:#fff;background-color: #3399FF;}";

    if(tablefocuspolicy == Qt::NoFocus) ui->tableView->setStyleSheet(style);
}

void SubTableWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(!movable) return;
    mpos = event->pos();
}

void SubTableWidget::mouseMoveEvent(QMouseEvent *event)
{
        QWidget::mouseMoveEvent(event);
        if(!movable) return;
        if (mpos.x() >= 0 && event->buttons() && Qt::LeftButton)
        {
            QPoint diff = event->pos() - mpos;
            QPoint newpos = this->pos() + diff;

            this->move(newpos);
        }
}

void SubTableWidget::mouseReleaseEvent(QMouseEvent *event)
{
        QWidget::mouseMoveEvent(event);
        if(!movable) return;
        /// очистить старое значение позиции нажатия мыши
        mpos = QPoint(-1, -1);
        QWidget::mouseReleaseEvent(event);
}

void SubTableWidget::setRow(int row)
{
    ui->tableView->selectionModel()->select(querymodel->index(row,0),
                                            QItemSelectionModel::ClearAndSelect |
                                            QItemSelectionModel::Rows);
}

void SubTableWidget::on_tableView_clicked(const QModelIndex &index)
{
    emit newrowselected(index.row() );                                          /// сообщить - новая строка выделена
}
