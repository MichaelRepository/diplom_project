/**
 Описание класса делегата.
 Делегат определяет каким образом будет отображаться
 элемент модели в вювере, а также какое виджет будет отображаться при
 изменении значения элемента модели.
 Также делегат содержит список строк для регулярных выражений,
 которые используются при создании валидатора данных каждого редактора,
 список передается делегату из вне методом setRegStrList.
**/

#ifndef RECORDDELEGATE_H
#define RECORDDELEGATE_H

#include <QDebug>

#include <QPainter>

#include <QStyledItemDelegate>
#include <QDateEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QRegExpValidator>
#include <QMap>

#include <QApplication>

#include "messdlg.h"
#include "dbmessdlg.h"
#include "subtablewidget.h"
#include "MyTable/mysqlrecord.h"

#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include <QRect>
/**
    кнопка, при нажатии которой отобразится виджет с субтаблицей
**/
class DelegatButton : public QPushButton
{
    Q_OBJECT
public:
    DelegatButton(int fieldnum,
                  MySqlRecord    *record,
                  QSqlQueryModel *model,
                  QWidget *parent):
        QPushButton(parent)
    {
        index = fieldnum;
        recorddata = record;
        querymodel = model;
        subtableshov = false;
        subtabledlg  = new SubTableWidget(this);
        subtabledlg->setTableModel(model);
        subtabledlg->setDisplayMode(false,false,false,Qt::NoFocus);
        QObject::connect(subtabledlg, &SubTableWidget::newrowselected,
                         this,        &DelegatButton:: update);

        /// позиционирование на текущей строке субтаблицы
        int row = 0;
        QString rv = recorddata->value(index).toString().toUpper();
        QSqlQuery query = model->query();
        query.first();
        do
        {
            QString qv = query.value(0).toString().toUpper();
            if(qv == rv) break;
            ++row;
        }
        while(query.next());
        subtabledlg->setRow(row);
        update(row);
    }

    void mouseReleaseEvent(QMouseEvent * event){                                /// отобразить виджет субтаблицы при отпускани кнопки
        showSubtable();
    }

    void paintEvent ( QPaintEvent * ){                                          /// переопределить прорисовку кнопки
       QStyleOptionButton option;
       option.initFrom(this);
       option.features = QStyleOptionButton::HasMenu;                           /// отобразить индикатор меню

       QPainter painter (this);
       style()->drawControl( QStyle::CE_PushButton, &option, &painter, this );  /// нарисовать кнопку
       QRect rect(this->rect().x()+5, this->rect().y()+2 ,
                  this->width(), this->height());
       painter.drawText(rect,this->text());                                     /// вывести текст
    }

    void showEvent(QShowEvent *event){
        QPushButton::showEvent(event);
        showSubtable();
    }

    void showSubtable()
    {
        if(subtableshov)
        {
            this->close();
            subtabledlg->hide();
            subtableshov = false;
            return;
        }
        subtabledlg->move(this->mapToGlobal( QPoint(0, this->height())) );
        subtabledlg->setWindowFlags(Qt::Popup);
        subtabledlg->setFocusPolicy(Qt::NoFocus);
        subtabledlg->setDisplayMode(false,false,false,Qt::NoFocus);
        subtabledlg->setFocusProxy(this);
        subtabledlg->resize(this->size().width()+70,200);
        subtabledlg->show();
        subtableshov = true;
    }

    void setcurrentrow(int row){subtablecurrentrow = row; subtabledlg->setRow(row);}

private slots:
    void update(int row)
    {
        subtablecurrentrow = row;
        querymodel->query().seek(subtablecurrentrow);

        QString  alterfield = recorddata->alterField(index);
        QVariant value      = querymodel->query().value(0);
        QVariant altervalue;
        recorddata->setValue (index, value);
        if(!alterfield.isEmpty() )
        {
            altervalue = querymodel->query().value(alterfield);
            recorddata->setAlterData(index, altervalue);
        }

        if(!alterfield.isEmpty()) this->setText(altervalue.toString());
        else this->setText(value.toString());

        this->repaint();
    }
private:
    int index;                   /// номер текущего поля
    SubTableWidget *subtabledlg; /// окно для отображения слинкованной таблицы
    QSqlQueryModel *querymodel;  /// модель, отображаемая в окне
    MySqlRecord    *recorddata;
    bool subtableshov;           /// состояние видимости окна
    int  subtablecurrentrow;     /// текущая выделенная строка в слинкованной таблице
};

class RecordDelegate : public QStyledItemDelegate
{
public:
    RecordDelegate(QString connectname, MySqlRecord *record, QObject * parent = 0);
    ~ RecordDelegate();

    void initDelegate();/// инициализация данных

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,/// метод создает редактор для каждого элемента модели
                          const QModelIndex & index) const;
    void	 setEditorData(QWidget * editor, const QModelIndex & index) const;  /// передает в редактор данные из модели
    void	 setModelData(QWidget * editor, QAbstractItemModel * model,         /// пердает данные из редактора в модель
                          const QModelIndex & index) const;
    void	paint(QPainter * painter, const QStyleOptionViewItem & option,
                  const QModelIndex & index) const;
private:
   MySqlRecord*  recorddata;
   QMap<int,QSqlQueryModel*> querylist; /// запросы на получение данных суб таблиц
   QString connectionname;
};

#endif // RECORDDELEGATE_H
