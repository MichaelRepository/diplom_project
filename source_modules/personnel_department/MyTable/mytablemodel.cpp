#include "mytablemodel.h"

MyTableModel::MyTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    QSize size(16,16);
    icoPKey.addFile(":/svg/key_icon.svg", size);
    icoFKey.addFile(":/svg/fkey-icon.svg",size);
    icoLink.addFile(":/svg/table_.svg",   size);
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

void MyTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}

int MyTableModel::columnCount(const QModelIndex &parent) const
{
    return table->displayedFieldsCount();
}

int MyTableModel::rowCount(const QModelIndex &parent) const
{
    return table->getRecordsCount();
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    bool isForeign = table->displayedFieldIsForeign(index.column());
    bool isPrimary = table->displayedFieldIsPrimary(index.column());
    bool isLink    = table->displayedFieldIsLink(index.column());

    switch (role) {
    case Qt::DisplayRole:
            return (table->displayCellValue(index.row(), index.column()) );

    case Qt::DecorationRole:
        if(isForeign) return icoFKey;
        if(isPrimary) return icoPKey;
        if(isLink)    return icoLink;
        return QVariant();
    default: return QVariant();
    }

}

QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

    return table->displayFieldName(section);
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
