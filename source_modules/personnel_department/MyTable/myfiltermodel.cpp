#include "myfiltermodel.h"

MyFilterModel::MyFilterModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    rootnode = 0;
    icon_condition = new QIcon(":/svg/filter-icon.svg");
    icon_group     = new QIcon(":/svg/filter_group_icon.svg");
    realoperators << "="
                  << "!="
                  << "<"
                  << "<="
                  << ">"
                  << ">="
                  << "LIKE"
                  << "NOT LIKE"
                  << "IS"
                  << "IS NOT";

    dispoperators << "равно"
                  << "не равно"
                  << "меньше"
                  << "меньше равно"
                  << "больше"
                  << "больше равно"
                  << "содержит"
                  << "не содержит"
                  << "пустой"
                  << "не пустой";

    realbinders << "AND" << "AND NOT" << "OR"  << "OR NOT" << "XOR";
    dispbinders << "И"   << "И НЕ"    << "ИЛИ" << "ИЛИ НЕ" << "XOR";
}

MyFilterModel::~MyFilterModel()
{

}

void MyFilterModel::setFilterData(MyFilterNode *root,
                                  QList<const MyField *> fieldlist)
{
    if(rootnode != 0)
    {
        disconnect(rootnode, &MyFilterNode ::modified,
                   this,     &MyFilterModel::filtermodified);
    }

    rootnode    = root;
    if(!rootnode) return;
    fields      = fieldlist;
    fieldsname.clear();

    connect(rootnode, &MyFilterNode ::modified,
            this,     &MyFilterModel::filtermodified);

    QList<const MyField *>::const_iterator itr;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        QString name = (*itr)->altername;
        if(name.isEmpty()) name = (*itr)->name;
        name = (*itr)->table->name+"."+name;
        fieldsname.append(name);
    }

    refresh();
}

void MyFilterModel::refresh()
{
    this->beginResetModel();
    this->endResetModel();
}

MyFilterNode *MyFilterModel::nodeFromIndex(const QModelIndex& index) const
{
    if(!rootnode || rootnode->isEmpty() ) return 0;
    if(!index.isValid()) return 0;
    MyFilterNode* node;
    if(! (node = static_cast<MyFilterNode*>(index.internalPointer()) ) )
        return 0;

    if(node) return node;
    else return 0;
}

QModelIndex MyFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!rootnode || row<0 || column <0 || column >= 4 || rootnode->isEmpty() )
        return QModelIndex();
    MyFilterNode * parentnode = nodeFromIndex(parent);
    if(!parent.isValid()) return createIndex(0,column,rootnode);
    Q_ASSERT(parentnode);
    if(!parentnode || parentnode->isEmpty() || row >= parentnode->childCount())
        return QModelIndex();
    MyFilterNode * node = parentnode->childAt(row);
    if(node) return createIndex(row, column, node);
    else return QModelIndex();
}

QModelIndex MyFilterModel::parent(const QModelIndex &child) const
{
    if(!rootnode || !child.isValid() || rootnode->isEmpty() ) return QModelIndex();
    MyFilterNode* node = nodeFromIndex(child);
    Q_ASSERT(node);
    if(!node || node == rootnode) return QModelIndex();
    MyFilterNode* parent = node->parentNode();
    if(!parent) return QModelIndex();
    Q_ASSERT(parent);
    if(parent == rootnode) return createIndex(0,0,rootnode);
    MyFilterNode* grandparent = parent->parentNode();
    Q_ASSERT(grandparent);
    if(!grandparent) return QModelIndex();
    int row = grandparent->indexOf(parent);
    return createIndex(row,0,parent);
}

int MyFilterModel::columnCount(const QModelIndex &parent) const
{
    if(!rootnode || rootnode->isEmpty() ) return 0;
    return 4;
}

int MyFilterModel::rowCount(const QModelIndex &parent) const
{
    if(!rootnode || rootnode->isEmpty()) return 0;
    if(!parent.isValid() ) return 1;
    MyFilterNode * parentnode = nodeFromIndex(parent);
    Q_ASSERT(parentnode);
    if(!parentnode ) return 0;
    if(parentnode) return parentnode->childCount();
    else return 0;
}

QVariant MyFilterModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || rootnode->isEmpty() ) return QVariant();
    MyFilterNode* node = nodeFromIndex(index);

    if(!node || (!node->isEmpty() && index.column() > 0) ) return QVariant();

    switch(role){
    case Qt::EditRole:{
        if(!node->isEmpty())
            return realbinders.indexOf(node->binder());
        else
        {
            switch(index.column())
            {
            case 0: return QVariant();
            case 1: return fieldIndexOf(node->table(), node->field());
            case 2: return realoperators.indexOf(node->foperator());
            case 3: return node->value();
            }
        }
    }
    case Qt::DisplayRole:{
        if(!node->isEmpty())
            return dispbinders.value(index.data(Qt::EditRole).toInt());
        else
        {
            switch(index.column())
            {
            case 0: return QVariant();
            case 1: return fieldsname.value(index.data(Qt::EditRole).toInt(),
                                            fieldsname[0]);
            case 2: return dispoperators.value(index.data(Qt::EditRole).toInt(), "равно");
            case 3: return node->value();
            }
        }
    }
    case Qt::UserRole:{
        if(!node->isEmpty())
            return dispbinders;
        else
        {
            switch(index.column())
            {
            case 0: return QVariant();
            case 1: return fieldsname;
            case 2: return dispoperators;
            case 3:{
                int num = fieldIndexOf(node);
                if(num == -1) return fields[0]->realtype;
                return fields[num]->realtype;
            }
            }
        }
    }
    case Qt::DecorationRole:{
        if( node->isEmpty() && index.column() == 0) return QVariant(*icon_condition);
        if(!node->isEmpty() && index.column() == 0) return QVariant(*icon_group);
        else return QVariant();
    }
    case Qt::SizeHintRole:{
        return QVariant();
    }
    default: return QVariant();

    }

}

QVariant MyFilterModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if(!rootnode || orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch(section){
    case 0: return QVariant("Связка");
    case 1: return QVariant("Поле");
    case 2: return QVariant("Оператор");
    case 3: return QVariant("Значение");
    }
}

bool MyFilterModel::setItemData(const QModelIndex &index,
                                const QMap<int, QVariant> &roles)
{
    bool res = true;
    QMap<int, QVariant>::const_iterator itr;
    for(itr = roles.begin(); itr != roles.end(); ++itr)
        res &= setData(index, itr.value(), itr.key());
    return res;
}

bool MyFilterModel::setData(const QModelIndex &index,
                            const QVariant &value,
                            int role)
{
    if(!index.isValid() || role != Qt::EditRole ) return false;
    MyFilterNode* node = nodeFromIndex(index);
    if(!node) return false;

    if(!node->isEmpty() && index.column() == 0)
    {
        node->setBinder(realbinders.value(value.toInt(), "AND") );
        return true;
    }

    switch (index.column()) {
    case 1:{
        const MyField * field = fields[value.toInt()];
        node->setTable(field->table->name);
        node->setField(field->name);
        return true;
    }
    case 2:{
        QString newvalue = realoperators.value(value.toInt(),"=");
        node->setOperator(newvalue);
        if(newvalue == "IS" || newvalue == "IS NOT") node->setValue("NULL");
        return true;
    }
    case 3: node->setValue(value.toString()); return true;
    default: return false;
    }
}

Qt::ItemFlags MyFilterModel::flags(const QModelIndex &index) const
{
    if(!index.isValid() ) return 0;
    MyFilterNode* node = nodeFromIndex(index);
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if( ( node->isEmpty() && index.column() >  0) ||
        (node != rootnode && !node->isEmpty() && index.column() == 0) )
         flags |= Qt::ItemIsEditable;
    return flags;
}

void MyFilterModel::addConditionsGroup(const QModelIndex &selectedindex)
{
    if(!rootnode || fields.size() == 0) return;
    beginResetModel();
    if(selectedindex.isValid())
    {
        MyFilterNode* node = nodeFromIndex(selectedindex);
        if(!node){endResetModel(); return;}
        if(node->isEmpty() ) node      = node->parentNode();
        MyFilterNode* newgroupnode     = new MyFilterNode(0,"AND");
        MyFilterNode* newgconditionode = new MyFilterNode(0,fields[0]->table->name,
                                                          fields[0]->name, "=","NULL");
        newgroupnode->addChild(newgconditionode);
        node->addChild(newgroupnode);
    }
    else {
        MyFilterNode* newgroupnode     = new MyFilterNode(0,"AND");
        MyFilterNode* newgconditionode = new MyFilterNode(0,fields[0]->table->name,
                                                          fields[0]->name, "=","NULL");
        newgroupnode->addChild(newgconditionode);
        rootnode->addChild(newgroupnode);
    }
    endResetModel();
}

void MyFilterModel::addCondition(const QModelIndex &selectedindex)
{
    if(!rootnode || fields.size() == 0) return;
    beginResetModel();
    if(selectedindex.isValid())
    {
        MyFilterNode* node = nodeFromIndex(selectedindex);
        if(!node){endResetModel(); return;}
        if(node->isEmpty() ) node      = node->parentNode();
        MyFilterNode* newgconditionode = new MyFilterNode(0,fields[0]->table->name,
                                                          fields[0]->name, "=","NULL");
        node->addChild(newgconditionode);
    }
    else {
        MyFilterNode* newgconditionode = new MyFilterNode(0,fields[0]->table->name,
                                                          fields[0]->name, "=","NULL");
        rootnode->addChild(newgconditionode);
    }
    endResetModel();
}

void MyFilterModel::removeChildItem(const QModelIndex &removableindex)
{
    if(!rootnode || fields.size() == 0) return;
    beginResetModel();
    if(removableindex.isValid())
    {
        MyFilterNode* node = nodeFromIndex(removableindex);
        if(node->parentNode()->childCount() == 1) node = node->parentNode();
        if(node == rootnode){ rootnode->clear(); endResetModel(); return;}
        MyFilterNode* parentnode = node->parentNode();
        int row = parentnode->indexOf(node);
        MyFilterNode* removednode = parentnode->takeChild(row);
        delete removednode;
    }
    endResetModel();
}

bool MyFilterModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return false;
}

bool MyFilterModel::insertRow(int row, const QModelIndex &parent)
{
    return true;
}

bool MyFilterModel::removeRow(int row, const QModelIndex &parent)
{
    return true;
}

void MyFilterModel::clearFilter()
{
    beginResetModel();
    rootnode->clear();
    endResetModel();
}

void MyFilterModel::filtermodified()
{
    refresh();
}

int MyFilterModel::fieldIndexOf(QString table, QString field) const
{
    table = table.toUpper();
    field = field.toUpper();
    QList<const MyField *>::const_iterator itr;
    int index = 0;
    for(itr = fields.begin(); itr != fields.end(); ++itr)
    {
        QString curtable = (*itr)->table->name.toUpper();
        QString curfield = (*itr)->name.toUpper();
        if(curtable == table && curfield == field) return index;
        ++index;
    }
    return -1;
}

int MyFilterModel::fieldIndexOf(MyFilterNode *node) const
{
    return fieldIndexOf(node->table(), node->field());
}
