#include "editrecordmodel.h"

EditRecordModel::EditRecordModel(QObject *parent) :
    QAbstractTableModel(parent){

}

EditRecordModel::~EditRecordModel()
{
    recorddata.clear();
}

void EditRecordModel::setData(QSqlRecord &record)
{
    recorddata = record;
}

int EditRecordModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;                                              /// вернуть число столбцов если индекс родительский (невалидный)
    return 2;
}

int EditRecordModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;
    return recorddata.count();
}

QVariant EditRecordModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() > recorddata.count())
        return QVariant();

    switch(role)                                                                /// относительно роли индекса возвращается значение
    {
    case Qt::DisplayRole:
    case Qt::EditRole:{
        if(recorddata.field(index.row()).value() =="...") return QVariant();    /// ОПРЕДЕЛЯЕТ КАКИЕ ПОЛЯ НЕ БУДУТ ОТОБРАЖАТЬСЯ В РЕДАКТОРЕ
        switch(index.column())
        {
        case 0: return recorddata.field(index.row()).name();                    /// название атрибута
        case 1: return recorddata.field(index.row()).value();                   /// значение атрибута
        default: return QVariant();
        }
    }
    case Qt::TextAlignmentRole:                                                 /// определяет размещение текста
        return Qt::AlignLeft;
    case Qt::BackgroundColorRole:
        return qVariantFromValue(QColor(255, 255, 191, 127));
    case Qt::CheckStateRole:                                                    /// определяет наличие переключателя
        return  QVariant();
    default: return QVariant();                                                 /// все остальные роли будут получать невалидный QVariant
    }
}

bool EditRecordModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || role != Qt::EditRole) return false;

    recorddata.field(index.row()).setValue(value);
    return true;
}

QVariant EditRecordModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section > 1 || role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch(section){
        case 0:     return QVariant("Атрибут");
        case 1:     return QVariant("Значение");
        default:    return QVariant();
        }
    }
    return section + 1;
}

Qt::ItemFlags EditRecordModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()) return 0;

    Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);

    switch(index.column()){
    case 0: theFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;                /// первый столбец - наименование атрибута (не редактируется)
        break;
    case 1: theFlags |= Qt::ItemIsSelectable| Qt::ItemIsEditable|               /// второй столбец - значение атрибутов (редактируемый)
                        Qt::ItemIsEnabled;
        break;
    default: theFlags = 0;
    }

    return theFlags;
}
