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
#include <QComboBox>
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
    DelegatButton(const QModelIndex& _index,
                  QSqlQueryModel *model,
                  QWidget *parent):
        QPushButton(parent)
    {
        index = _index;
        querymodel = model;
        subtableshov = false;
        subtabledlg  = new SubTableWidget(this);
        subtabledlg->setTableModel(querymodel);
        subtabledlg->setDisplayMode(false,false,false,Qt::NoFocus);
        QObject::connect(subtabledlg, &SubTableWidget::newrowselected,
                         this,        &DelegatButton:: update);

        /// позиционирование на текущей строке субтаблицы
        int row = 0;
        QString rv = index.data(Qt::DisplayRole).toString().toUpper();
        QSqlQuery query = querymodel->query();
        query.first();
        do
        {
            QString qv = query.value(0).toString().toUpper();
            if(qv == rv) break;
            ++row;
        }
        while(query.next());
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
    QVariant realValue ()const  {return realvalue; }
    QVariant alterValue()const  {return altervalue.isNull() ? realvalue : altervalue; }

private slots:
    void update(int row)
    {
        subtablecurrentrow = row;
        if(!querymodel->query().seek(subtablecurrentrow) )
            subtablecurrentrow = 0;

        subtabledlg->setRow(subtablecurrentrow);

        QString  alterfield = index.data(Qt::UserRole+4).toString();
        realvalue = querymodel->query().value(0);
        if(!alterfield.isEmpty() )
        {
            altervalue = querymodel->query().value(alterfield);
            this->setText(altervalue.toString());
        }
        else this->setText(realvalue.toString());

        this->repaint();
    }

private:
    QModelIndex index;
    SubTableWidget *subtabledlg; /// окно для отображения слинкованной таблицы
    QSqlQueryModel *querymodel;  /// модель, отображаемая в окне
    bool subtableshov;           /// состояние видимости окна
    int  subtablecurrentrow;     /// текущая выделенная строка в слинкованной таблице

    QVariant realvalue;
    QVariant altervalue;
};

class RecordDelegate : public QStyledItemDelegate
{
public:
    RecordDelegate(QString connectname, QObject * parent = 0);
    ~ RecordDelegate();

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,/// метод создает редактор для каждого элемента модели
                          const QModelIndex & index) const;
    void	 setEditorData(QWidget * editor, const QModelIndex & index) const;  /// передает в редактор данные из модели
    void	 setModelData(QWidget * editor, QAbstractItemModel * model,         /// передает данные из редактора в модель
                          const QModelIndex & index) const;
    void	 paint(QPainter * painter, const QStyleOptionViewItem & option,
                   const QModelIndex & index) const;

private:
   QMap<int, QSqlQueryModel*> * querylist; /// запросы на получение данных суб таблиц
   QString connectionname;
};

#endif // RECORDDELEGATE_H
