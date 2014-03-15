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

#include <QStyledItemDelegate>
#include <QPainter>
#include <QDateEdit>
#include <QLineEdit>
#include <QRegExpValidator>

class RecordDelegate : public QStyledItemDelegate
{
public:
    RecordDelegate();
    ~ RecordDelegate();

    void setRegStrList(QList<QString> *list);                                   /// установить список рег. выражений для каждого валидатора данных

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,/// метод создает редактор для каждого элемента модели
                          const QModelIndex & index) const;
    void	 setEditorData(QWidget * editor, const QModelIndex & index) const;  /// передает в редактор данные из модели
    void	 setModelData(QWidget * editor, QAbstractItemModel * model,         /// пердает данные из редактора в модель
                          const QModelIndex & index) const;
    // void	paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
private:
    QList<QString> *regexplist;                                                 /// список регулярных выражений для валидаторов
};

#endif // RECORDDELEGATE_H
