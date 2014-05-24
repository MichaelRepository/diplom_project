#include "myeditrecordmodel.h"

EditRecordModel::EditRecordModel(QObject *parent) :
    QAbstractTableModel(parent){

}

EditRecordModel::~EditRecordModel()
{

}

void EditRecordModel::setRecordData(MySqlRecord *record)
{
    this->beginResetModel();
    recorddata = record;
    this->endResetModel();
}

int EditRecordModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;
    return 2;
}

int EditRecordModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;
    return recorddata->count();
}

QVariant EditRecordModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() > recorddata->count())
        return QVariant();

    /* изменения в работе модели и делегата.
     * - необходимо определить реальный тип данных поля
     * - в displayrole выдавать значение поля
     * - в editrole передавать значение, либо его варианты
     * - в userrole передавать реальный тип данных поля
     * - в userrole+1 передавать валидатор поля
     * - в userrole+2 передавать текст sql запроса, если поле - внешний ключ
     * - в userrole+3 передать список альтернативных имен для полей таблицы
     *   получаемой из sql запроса (из userrole+2)
     * - в userrole+4 передать имя поля, замещающего текущее !!! (альтернативное поле)
    */

    QString realtype = recorddata->realTypeOfField(index.row() );
    if(realtype.isEmpty() ) return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:{
        if(index.column() == 0 )
            return QVariant(recorddata->alterNameOfField(index.row()) );
        else
        {
                /*if(recorddata->alterField(index.row()).isEmpty() )
                    return recorddata->value(index.row());
                else
                    return recorddata->alterData(index.row());*/

                if(realtype == "TINYINT")
                {
                    if(recorddata->value(index.row() ).toBool() ) return "да";
                    else return "нет";
                }
                else return recorddata->value(index.row() );
        }
    }
    case Qt::EditRole:{

        if(index.column() == 0) return QVariant();
        else
        {
            /*if(recorddata->alterField(index.row()).isEmpty() )
                return recorddata->value(index.row());
            else
                return recorddata->alterData(index.row());*/

            if(realtype == "TINYINT")
            {
                QStringList list;
                list << "да" << "нет";
                return list;
            }
            else if(realtype == "ENUM")
                return recorddata->getEnumVariants(index.row() );
            else
                return recorddata->value(index.row() );
        }
    }
    case Qt::UserRole:{ /// передать тип переменной, стандарта СУБД
        if(index.column() == 0)
            return QVariant();
        else
            return realtype;
    }
    case Qt::UserRole+1:{/// передать рег. выражение для валидатора
        if(index.column() == 0)
            return QVariant();
        else
            return recorddata->validatorOfField(index.row() );
    }
    case Qt::UserRole+2:{/// передать текст SQL запроса
        if(index.column() == 0)
            return QVariant();
        else
        {
            MyDataReference reference = recorddata->referenceDataOfField(index.row() );
            return reference.sqltext;
        }
    }
    case Qt::UserRole+3:{/// передать список альтернативных имен для полей
        if(index.column() == 0)
            return QVariant();
        else
        {
            MyDataReference reference = recorddata->referenceDataOfField(index.row() );
            return reference.alternames;
        }
    }
    case Qt::UserRole+4:{/// передать имя поля, замещающего текущее
        if(index.column() == 0)
            return QVariant();
        else
            return recorddata->alterField(index.row() );
    }
    default: return QVariant();
    }
}

bool EditRecordModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || role != Qt::EditRole) return false;

    QString realtype = recorddata->realTypeOfField(index.row() );
    if(realtype.isEmpty() ) return false;

    if(realtype == "TINYINT" )
    {
        if(value.toString() == "да")
            recorddata->setValue(index.row(), 1);
        else
            recorddata->setValue(index.row(), 0);
    }
    else
        recorddata->setValue(index.row(), value);

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
}

Qt::ItemFlags EditRecordModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()) return 0;
    Qt::ItemFlags theFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if(index.column() == 1) theFlags |= Qt::ItemIsEditable;
    return theFlags;
}
