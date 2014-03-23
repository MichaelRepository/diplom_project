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
#include "subtablewidget.h"

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
    DelegatButton(QWidget *parent):QPushButton(parent){
        subtabledlg = new SubTableWidget(this);
        QObject::connect(subtabledlg, &SubTableWidget::newrowselected,
                         this,        &DelegatButton::update);
    }
// ЗДЕСЬ //
    void updateDate(){                                                          /// перезаполучить данные у виджета субтаблицы
        QVariant value = subtabledlg->getDisplayedFieldValue();
        this->setText(value.toString());
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
        if(subtableshov){
            this->close();
            subtabledlg->hide();
            subtableshov = false;
            return;
        }
        subtabledlg->move(this->mapToGlobal( QPoint(0, this->height())) );
        subtabledlg->setWindowFlags(Qt::Popup);
        subtabledlg->setFocusPolicy(Qt::NoFocus);
        subtabledlg->setDisplayMode(false,false,false,false,Qt::NoFocus);
        subtabledlg->setFocusProxy(this);
        subtabledlg->resize(this->size().width()+70,200);
        subtabledlg->show();
        subtableshov = true;
    }

private slots:
    void update(){updateDate(), this->repaint();}
private:
    SubTableWidget *subtabledlg;
    bool subtableshov;
};

class RecordDelegate : public QStyledItemDelegate
{
public:
    RecordDelegate(QObject * parent = 0);
    ~ RecordDelegate();

    void setRegStrList        (QList<QString> *list);                           /// установить список рег. выражений для каждого валидатора данных
    void setTableAttributeList(QMap<QString, SubTableWidget *> *list);          /// установить список атрибутов, являющихся субтаблицами

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,/// метод создает редактор для каждого элемента модели
                          const QModelIndex & index) const;
    void	 setEditorData(QWidget * editor, const QModelIndex & index) const;  /// передает в редактор данные из модели
    void	 setModelData(QWidget * editor, QAbstractItemModel * model,         /// пердает данные из редактора в модель
                          const QModelIndex & index) const;
    void	paint(QPainter * painter, const QStyleOptionViewItem & option,
                  const QModelIndex & index) const;
private:
    QList<QString> *regexplist;                                                 /// список регулярных выражений для валидаторов
    QMap<QString, SubTableWidget *> *tableattributelist;                        /// список атрибутов, являющихся субтаблицами
};

#endif // RECORDDELEGATE_H
