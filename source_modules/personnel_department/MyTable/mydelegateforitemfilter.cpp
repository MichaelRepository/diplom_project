#include "mydelegateforitemfilter.h"

MyDelegateForItemFilter::MyDelegateForItemFilter()
{
}

QWidget *MyDelegateForItemFilter::createEditor(QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    QVariant::Type itemtype  = index.data(Qt::UserRole).type();

    switch (itemtype) {
    case QVariant::StringList:{
            QComboBox * combobox = new QComboBox(parent);
            combobox->addItems(index.data(Qt::UserRole).toStringList());
            combobox->setCurrentIndex(index.data(Qt::EditRole).toInt());
            return combobox;
        }
    case QVariant::String: {
        QString realtype = index.data(Qt::UserRole).toString().toUpper();
        //if(realtype == "")
        QLineEdit *lineedit = new QLineEdit(parent);
        lineedit->setText(index.data(Qt::DisplayRole).toString());
        return lineedit;
    }
    default: return 0;
    }
}

void MyDelegateForItemFilter::setEditorData(QWidget *editor,
                                            const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor,index);
}

void MyDelegateForItemFilter::setModelData(QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index) const
{
    QVariant::Type itemtype  = index.data(Qt::UserRole).type();

    switch (itemtype) {
    case QVariant::StringList:{
            QComboBox * combobox = static_cast<QComboBox *>(editor);
            int value = combobox->currentIndex();

            QMap<int, QVariant> roles;
            roles.insert(Qt::EditRole, value);
            model->setItemData(index, roles);
            return;
        }
    case QVariant::String: {
        QString newvalue;
        QString realtype = index.data(Qt::UserRole).toString().toUpper();
        //if(realtype == "")
        QLineEdit *lineedit = static_cast<QLineEdit*> (editor);
        newvalue = lineedit->text();
        if(newvalue.isEmpty()) newvalue = "'NULL'";
        else{
            if(newvalue[0] != '\'')           newvalue  = "'"+newvalue;
            if(newvalue[newvalue.length()-1] != '\'') newvalue += "'";
        }
        QMap<int, QVariant> roles;
        roles.insert(Qt::EditRole, newvalue);
        model->setItemData(index, roles);
        return ;
    }
    }
}

void MyDelegateForItemFilter::paint(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize MyDelegateForItemFilter::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

void MyDelegateForItemFilter::updateEditorGeometry(QWidget *editor,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index) const
{
    if(index.column() == 0)
    {
        editor->move(option.rect.x()-3, option.rect.y());
        editor->resize(option.rect.width()+3, option.rect.height());
    }
    else{
        editor->setGeometry(option.rect);
    }
}
