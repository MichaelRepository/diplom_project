#include "mytablemodel.h"

MyTableModel::MyTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    QSize size(16,16);
    icoPKey.addFile(":/png/iconmonstr-key-icon-16.png", size);
    icoFKey.addFile(":/png/iconmonstr-key-3-icon-16.png",size);
    icoLink.addFile(":/png/table_.png",   size);
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
    if(parent.isValid()) return 0;
    return table->getRecordsCount();
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    bool isForeign  = table->displayedFieldIsForeign(index.column() );
    bool isPrimary  = table->displayedFieldIsPrimary(index.column() );
    bool isLink     = table->displayedFieldIsLink(index.column() );
    QString realtype = table->displayFieldRealType(index.column() );

    switch (role) {
    case Qt::DisplayRole:{
        QVariant cellvalue = table->displayCellValue(index.row(), index.column());
        if(realtype == "TINYINT")
        {
            if(cellvalue.toBool()) return "да";
            else return "нет";
        }
        else
            return cellvalue;
    }
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

    QString sectionname = table->displayFieldAlterName(section);
    if(sectionname.isEmpty()) sectionname = table->displayFieldName(section);

    return sectionname;
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
