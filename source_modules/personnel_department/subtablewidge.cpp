#include "subtablewidget.h"
#include "ui_subtablewidget.h"

WidgetEditRecord::WidgetEditRecord(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetEditRecord)
{
    ui->setupUi(this);
}

WidgetEditRecord::~WidgetEditRecord()
{
    delete ui;
}
