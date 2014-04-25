#ifndef MYFILTERMODEL_H
#define MYFILTERMODEL_H

#include <QAbstractItemModel>
#include <QFontMetrics>
#include "myfilterdata.h"

class MyFilterModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit MyFilterModel(QObject *parent = 0);
    ~MyFilterModel();
    void setFilterData(MyFilterNode *root, QList<const MyField *> fieldlist);
    void refresh();

    MyFilterNode* nodeFromIndex(const QModelIndex& index)const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int columnCount (const QModelIndex & parent = QModelIndex()) const;         /// возвращает кол-во столбцов относительно родительского индекса
    int rowCount    (const QModelIndex & parent = QModelIndex()) const;         /// возвращает число строк относительно родительского индекса
    QVariant data   (const QModelIndex & index, int role = Qt::DisplayRole) const; /// отображение данных текущего индекса, относительно роли объекта
    QVariant headerData ( int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole ) const;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    bool setData(const QModelIndex & index, const QVariant & value,
                 int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;                        /// установка флагов (активный, выделяемый и т.д)

    void addConditionsGroup(const QModelIndex &selectedindex);
    void addCondition      (const QModelIndex &selectedindex);
    void removeChildItem   (const QModelIndex &removableindex);

    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role);
    bool insertRow(int row, const QModelIndex &parent);
    bool removeRow(int row, const QModelIndex &parent);
    void clearFilter();


private slots:
    void filtermodified();

private:
    int fieldIndexOf(QString table, QString field) const;
    int fieldIndexOf(MyFilterNode *node) const;

    QIcon* icon_condition;
    QIcon* icon_group;

    MyFilterNode *rootnode;
    QList<const MyField *> fields;

    QStringList realbinders;                                                    /// связующие элементы
    QStringList dispbinders;
    QStringList realoperators;                                                  /// логические операторы
    QStringList dispoperators;

    QStringList fieldsname;
};

#endif // MYFILTERMODEL_H
