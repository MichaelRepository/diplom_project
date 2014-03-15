/**
    Модель данных для редактора записи таблицы.
    Модель позволяет работать с данными одной записи запроса, хроян ее
    в списке пары элементов: <атрибут,значение>.
    Атрибут доступен только для чтения, а значение для редактирования.
    Модель представлена в виде табличное модели, состоящий только из двух
    столбцов(атрибут, значение). Число строк соответсвует числу атрибутов
    таблицы запроса.
**/
#ifndef EDITRECORDMODEL_H
#define EDITRECORDMODEL_H

#include <QAbstractTableModel>
#include <QColor>

#include <QList>
#include <QPair>
#include <QVariant>

class EditRecordModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit EditRecordModel(QObject *parent = 0);
    ~EditRecordModel();

    int columnCount(const QModelIndex & parent = QModelIndex()) const;          /// возвращает кол-во столбцов относительно родительского индекса
    int rowCount(const QModelIndex & parent = QModelIndex()) const;             /// возвращает число строк относительно родительского индекса
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const; /// отображение данных текущего индекса, относительно роли объекта
    bool setData(const QModelIndex & index, const QVariant & value,             /// записывает данные в элемент соответсвующий индексу
                 int role = Qt::EditRole);
    QVariant headerData ( int section, Qt::Orientation orientation,             /// данные в зоголовках столбцов
                          int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;                        /// установка флагов (активный, выделяемый и т.д)

    void modelAddRow(QString key, QVariant value);                              /// метод для добавлении данных
    QVariant getValue(int row);                                                 /// метод для получения значения по номеру атрибута

private:
    QList< QPair<QString, QVariant> > listmodel;
};

#endif // EDITRECORDMODEL_H
