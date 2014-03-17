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
#include <QLabel>
#include <QDateEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QRegExpValidator>
#include <QVector>
#include <QMap>

#include <QApplication>

#include "messdlg.h"
#include "subtabledialog.h"

#include <QToolButton>
#include <QTableView>
/**
    кнопка, при нажатии которой отобразится субтаблица
**/
class DelegatButton : public QToolButton
{
    Q_OBJECT
public:
    DelegatButton(QWidget *parent):QToolButton(parent){
        QObject::connect(this,&DelegatButton::clicked,
                         this,&DelegatButton::click);
    }

    void setSubTableDialog(SubTableDialog *dlg){
        subtabledlg = dlg;
        subtabledlg->setWindowFlags(Qt::Popup);
    }

public slots:
    void click(){
        QPoint newpos( this->rect().x(), this->rect().y()+this->height() );
        subtabledlg->resize(this->rect().width(), 200);
        subtabledlg->move(this->mapToGlobal(newpos) );

        subtabledlg->exec();
    }

private:
    SubTableDialog *subtabledlg;
};



class RecordDelegate : public QStyledItemDelegate
{
public:
    RecordDelegate(QObject * parent = 0);
    ~ RecordDelegate();

    void setRegStrList        (QList<QString> *list);                           /// установить список рег. выражений для каждого валидатора данных
    void setTableAttributeList(QMap<QString, SubTableDialog *> *list);          /// установить список атрибутов, являющихся субтаблицами

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,/// метод создает редактор для каждого элемента модели
                          const QModelIndex & index) const;
    void	 setEditorData(QWidget * editor, const QModelIndex & index) const;  /// передает в редактор данные из модели
    void	 setModelData(QWidget * editor, QAbstractItemModel * model,         /// пердает данные из редактора в модель
                          const QModelIndex & index) const;
    /*void     updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                  const QModelIndex &) const;*/

    void	paint(QPainter * painter, const QStyleOptionViewItem & option,
                  const QModelIndex & index) const;
private:
    QList<QString> *regexplist;                                                 /// список регулярных выражений для валидаторов
    QMap<QString, SubTableDialog *> *tableattributelist;                        /// список атрибутов, являющихся субтаблицами
};

#endif // RECORDDELEGATE_H
