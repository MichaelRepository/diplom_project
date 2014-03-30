#include "mytablemodel.h"

MyTableModel::MyTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

MyTableModel::~MyTableModel()
{

}

void MyTableModel::setTableObject(MyTable *tableobj)
{
    beginResetModel();
    table = tableobj;
    endResetModel();
}

int MyTableModel::columnCount(const QModelIndex &parent) const
{
    return table->getFieldsCount();
}

int MyTableModel::rowCount(const QModelIndex &parent) const
{
    return table->getRecordsCount();
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return (table->getCellValue(index.row(), index.column()) );
    default: return QVariant();
    }

}

QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

    return table->getFieldName(section);
}

bool MyTableModel::setRecordData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags MyTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()) return 0;
    Qt::ItemFlags theFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return theFlags;
}
