#ifndef MYDELEGATEFORITEMFILTER_H
#define MYDELEGATEFORITEMFILTER_H

#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QComboBox>

#include "mysqlfield.h"

class MyDelegateForItemFilter : public QStyledItemDelegate
{
public:
    MyDelegateForItemFilter();

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,/// метод создает редактор для каждого элемента модели
                          const QModelIndex & index) const;
    void	 setEditorData(QWidget * editor, const QModelIndex & index) const;  /// передает в редактор данные из модели
    void	 setModelData(QWidget * editor, QAbstractItemModel * model,         /// пердает данные из редактора в модель
                          const QModelIndex & index) const;
    void	paint(QPainter * painter, const QStyleOptionViewItem & option,
                  const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // MYDELEGATEFORITEMFILTER_H
