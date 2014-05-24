#include "subtablewidget.h"
#include "ui_subtablewidget.h"

SubTableWidget::SubTableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubTableWidget)
{
    ui->setupUi(this);

    statusbar       = new QStatusBar(this);
    movable         = true;
    ui->label->     setVisible    (true);
    ui->toolsframe->setVisible    (true);
    ui->tableView-> setFocusPolicy(Qt::NoFocus);

    QLayout* layoutWidget = ui->gridLayout_2;
    layoutWidget->setContentsMargins(0,0,0,0);
    layoutWidget->setSpacing(0);
    layoutWidget->addWidget(statusbar);
}

SubTableWidget::~SubTableWidget()
{
    delete ui;
}

void SubTableWidget::setTableModel(QAbstractItemModel *model)
{
   tablemodel = model;
   ui->tableView->setModel(tablemodel);
   ui->tableView->resizeColumnsToContents();
   ui->tableView->resizeRowsToContents();
}

void SubTableWidget::setTitleText(QString text)
{
    ui->label->setText(text);
}

void SubTableWidget::setDisplayMode(bool titleVisible,
                                    bool menuPanelVisible,
                                    bool dlgMovable,
                                    Qt::FocusPolicy tablefocuspolicy)
{
    movable = dlgMovable;
    ui->label->     setVisible    (titleVisible);
    ui->toolsframe->setVisible    (menuPanelVisible);
    ui->tableView-> setFocusPolicy(tablefocuspolicy);
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
    ui->tableView->selectionModel()->select(tablemodel->index(row,0),
                                            QItemSelectionModel::ClearAndSelect |
                                            QItemSelectionModel::Rows);
}

int SubTableWidget::getSelectedRowIndex()
{
    return ui->tableView->selectionModel()->selectedRows(0).first().row();
}

QList<int> SubTableWidget::selectedRows() const
{
    QList<int> rows;
    QModelIndexList items = ui->tableView->selectionModel()->selectedRows(0);
    QModelIndexList::const_iterator itri;
    for(itri = items.begin(); itri != items.end(); ++itri) rows << (*itri).row();
    return rows;
}

void SubTableWidget::show()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
    QWidget::show();
}

void SubTableWidget::on_tableView_clicked(const QModelIndex &index)
{
    emit newrowselected(index.row() );                                          /// сообщить - новая строка выделена
}

void SubTableWidget::on_Edit_button1_clicked()
{
    emit editButtonClicked();
}

void SubTableWidget::on_Add_button1_clicked()
{
    emit addButtonClicked();
}

void SubTableWidget::on_Delete_button1_clicked()
{
    emit removeButtonClicked();
}
