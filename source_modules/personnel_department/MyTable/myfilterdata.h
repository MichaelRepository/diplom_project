#ifndef MYFILTERDATA_H
#define MYFILTERDATA_H

#include <QObject>
#include "mysqlfield.h"

class MyFilterNode: public QObject
{
    Q_OBJECT
public:
    MyFilterNode(MyFilterNode* nodeparent = 0, QString binder = "AND",
                 QObject* objparent = 0): QObject(objparent)
    {
        parent   = nodeparent;
        f_binder = binder;
    }
    MyFilterNode(MyFilterNode* nodeparent,QString table, QString field,
                 QString foperator = "=", QString value = "NULL",
                 QObject* objparent = 0): QObject(objparent)
    {
        parent      = nodeparent;
        f_table     = table;
        f_field     = field;
        f_operator  = foperator;
        f_value     = value;
    }
    ~MyFilterNode() { qDeleteAll(childs); }

    QString binder()    const {return f_binder;}
    QString table()     const {return f_table;}
    QString field()     const {return f_field;}
    QString foperator() const {return f_operator;}
    QString value()     const {return f_value;}
    void setBinder  (const QString &binder)   {f_binder = binder;}
    void setTable   (const QString &table)    {f_table  = table;}
    void setField   (const QString &field)    {f_field  = field;}
    void setValue   (const QString &value)    {f_value  = value;}
    void setOperator(const QString &foperator){f_operator = foperator;}

    int  childCount() const {return childs.count();}
    bool isEmpty()    const {return childs.isEmpty();}
    int  indexOf(MyFilterNode* node) const{
        return childs.indexOf(node);
    }

    void clear(){
        emit modified();
        if(!childs.isEmpty())
        {
            qDeleteAll(childs);
            childs.clear();
        }
        f_binder    = "AND";
        f_operator  = QString();
        f_table     = QString();
        f_field     = QString();
        f_value     = QString();
    }

    MyFilterNode* parentNode()       const {return parent;}
    MyFilterNode* childAt(int index) const {return childs.value(index,0);}
    QList<MyFilterNode*> childrens() const {return childs;}

    void  insertChildNode(int index, MyFilterNode* node){
        emit modified();
        node->parent = this;
        childs.insert(index, node);
    }

    void addChild(MyFilterNode* node){
        emit modified();
        connect(node, &MyFilterNode::modified,
                this, &MyFilterNode::modified);
        node->parent = this;
        childs.append(node);
    }

    MyFilterNode* takeChild(int index)
    {
        emit modified();
        MyFilterNode* item = childs.takeAt(index);
        disconnect(item, &MyFilterNode::modified,
                   this, &MyFilterNode::modified);
        Q_ASSERT(item);
        item->parent = 0;
        return item;
    }

    QString filterConditionText() const { return nodeToFilterConditionText(this); }
signals:
    void modified();                                                            /// сообщвет об изменениях

private:

    QString nodeToFilterConditionText(const MyFilterNode* node) const
    {
        QString result;

        if(node->isEmpty())
        {
            result += " "+node->table()+"."+node->field()+" "+
                      node->foperator()+" "+node->value()+" ";

            return result;
        }

        for(int i = 0; i < node->childCount(); ++i)
        {
            MyFilterNode* child = node->childAt(i);
            if(child->isEmpty())
            {
                if(i > 0) result += " AND ";
                result += " "+child->table()+"."+child->field()+" "+
                          child->foperator()+" "+child->value()+" ";
            }
            else
            {
                if(node->indexOf(child) > 0) result +=  " "+child->binder()+" ";
                result += " ( "+nodeToFilterConditionText(child)+") ";
            }
        }
        return result;
    }

    QString  f_binder;                                                          /// связующий элемент
    QString  f_table;                                                           /// таблица
    QString  f_field;                                                           /// полей
    QString  f_operator;                                                        /// логический оператор
    QString  f_value;                                                           /// значение узла фильтра

    MyFilterNode*        parent;                                                /// родительский узел
    QList<MyFilterNode*> childs;                                                /// дочерние узлы
};

class MySelectStruct{
public:
    MySelectStruct() {rootofwhere = new MyFilterNode(); filterroot = 0;}
    ~MySelectStruct(){delete rootofwhere;}

    void addSelectItem(const QString& item){ selectitems.append(item); }
    void addFromItem  (const QString& item){ fromitems.append(item);}
    void addOrderItem (const QString& item){ orderitems.append(item);}
    void addWehereNode(MyFilterNode* node)
    {
        rootofwhere->addChild(node);
        copyTablesFromNode(node, fromitems);
    }

    MyFilterNode* rootOfWhere() const {return rootofwhere;}

    void setFilter(MyFilterNode* filter_root){ filterroot = filter_root; }
    void removeFilter(){ filterroot = 0; }

    QString selectQueryText(bool filteractive = false) const
    {
        QString result;
        result = "SELECT ";
        if(selectitems.size() == 0)
            result += " * ";
        else
            result += selectitems.join(',');

        if(fromitems.size() == 0) return QString();
        result += " FROM " + fromitems.join(',');

        if(!rootofwhere->isEmpty())
            result += " WHERE " + rootofwhere->filterConditionText();
        /// добавление данных фильтра
        if(filteractive && filterroot && !filterroot->isEmpty() )
        {
            QString filtertext = filterroot->filterConditionText();
            if(rootofwhere->isEmpty())
                result += " WHERE "+filtertext;
            else
                result += " AND ("+filtertext+") ";
        }

        if(customOrder.size() > 0)
        {
            result += " ORDER BY ";
            QMap<int, QString>::const_iterator itr;
            for(itr = customOrder.begin(); itr != customOrder.end(); ++itr)
            {
                if(itr != customOrder.begin() ) result += ", ";
                if(itr.value() == "ASC" || itr.value() == "DESC")
                    result += " "+QString::number(itr.key() )+" "+itr.value();
            }
        }
        else if(orderitems.size() > 0)
            result += " ORDER BY " + orderitems.join(',');

        //qDebug() << result <<" << ";

        return result;
    }

    QString getOrderItem(int index) const
    {
        if(index < 0 || index >= orderitems.size() ) return QString();
        return orderitems[index];
    }

    QMap<int, QString> getCustomOrdersData() const
    {
        QMap<int, QString> result;
        QMap<int, QString>::const_iterator itr;
        for(itr = customOrder.begin(); itr != customOrder.end(); ++itr)
        {
            if(itr.value() == "ASC" || itr.value() == "DESC")
                result.insert(itr.key(), itr.value() );
        }
        return result;
    }

    void setCustomOrderData(int column, QString data)
    {
        if(customOrder.contains(column) ) customOrder[column] = data;
        else customOrder.insert(column, data);
    }

    void clearCustomOrdersData()
    {
        customOrder.clear();
    }

    bool customOrderIsEmpty() const {return customOrder.isEmpty(); }

private:

    void copyTablesFromNode(MyFilterNode* node, QStringList& tables)
    {
        if(node->isEmpty())
        {
            QString table = node->table();
            if(!tables.contains(table)) tables.append(table);
        }

        for(int i = 0; i < node->childCount(); ++i)
        {
            MyFilterNode* child = node->childAt(i);
            if(child->isEmpty())
            {
                QString table = node->table();
                if(!tables.contains(table)) tables.append(table);
            }
            else{
                copyTablesFromNode(child, tables);
            }
        }
    }

    QStringList   selectitems;
    QStringList   fromitems;
    QStringList   orderitems;                                                   /// порядок устанавливаемый при стандартном запросе
    MyFilterNode* rootofwhere;
    MyFilterNode* filterroot;

    QMap<int, QString> customOrder;                                             /// порядок столбцов установленный пользователем

};


#endif // MYFILTERDATA_H
