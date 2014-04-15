#ifndef MYTABLEMODEL_H
#define MYTABLEMODEL_H

#include <QAbstractTableModel>
#include "mytable.h"

class MyTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MyTableModel(QObject *parent = 0);
    ~MyTableModel();

    void setTableObject(MyTable* tableobj);
    void refresh();

    int columnCount (const QModelIndex & parent = QModelIndex()) const;         /// возвращает кол-во столбцов относительно родительского индекса
    int rowCount    (const QModelIndex & parent = QModelIndex()) const;         /// возвращает число строк относительно родительского индекса
    QVariant data   (const QModelIndex & index, int role = Qt::DisplayRole) const; /// отображение данных текущего индекса, относительно роли объекта
    QVariant headerData ( int section, Qt::Orientation orientation,             /// данные в заголовках столбцов
                          int role = Qt::DisplayRole ) const;
    bool setRecordData    (const QModelIndex & index, const QVariant & value,         /// записывает данные в элемент соответсвующий индексу
                     int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;                        /// установка флагов (активный, выделяемый и т.д)

private:
    MyTable* table;
    QIcon icoPKey;
    QIcon icoFKey;
    QIcon icoLink;
};

    /*bool setData    (const QModelIndex & index, const QVariant & value,         /// записывает данные в элемент соответсвующий индексу
                     int role = Qt::EditRole);*/
#endif // MYTABLEMODEL_H
