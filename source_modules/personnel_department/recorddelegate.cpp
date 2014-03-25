#include "recorddelegate.h"

RecordDelegate::RecordDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{

}

RecordDelegate::~RecordDelegate()
{

}

void RecordDelegate::setTable(MyTable *_table)
{
    table = _table;
}

QWidget *RecordDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определяет тип данных
    QModelIndex atributeindex = index.model()->index(index.row(),0);
    QString curatribut = atributeindex.data().toString();

    /// проверка является ли атрибут внешним ключем (ссылающимся на внешнюю таблицу)
    if(table->isForeignKey(index.row()))
    {
            /// вернуть в качестве редактора диалог вывода субтаблицы
            MyTable* foreigntable = table->getForeignTable(index.row());
            DelegatButton *button = new DelegatButton(foreigntable,parent);
            return button;
    }

    switch (curtype) {
    case QVariant::Date:{
        QDateEdit* dataedit = new QDateEdit(parent);                            /// редактор - календарь
        dataedit->setCalendarPopup(true);
        dataedit->setFixedSize(option.rect.width(),option.rect.height());
        return dataedit;
    }
    case QVariant::String:{
        QLineEdit* lineedit = new QLineEdit(parent);                            /// редактор - поле ввода
        lineedit->setFixedSize(option.rect.width(),option.rect.height());
        return lineedit;
    }
    case QVariant::Int:{
        QLineEdit* lineedit = new QLineEdit(parent);                            /// редактор - поле ввода
        lineedit->setFixedSize(option.rect.width(),option.rect.height());
        return lineedit;
    }
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);        /// выбор за QStyledItemDelegate
    }

}

void RecordDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определяет тип данны
    QModelIndex atrivuteindex = index.model()->index(index.row(),0);
    QString curatribut = atrivuteindex.data().toString();
    /// проверка является ли атрибут субтаблицей
    if(table->isForeignKey(index.row()))
    {
        DelegatButton *button = static_cast<DelegatButton*>(editor);
        button->updateDate();
        return ;
    }


    /// создание валидатора для текущего редактора
    QString field = index.model()->index(index.row(),0).data(Qt::EditRole).toString();
    QString regstr = table->getFieldValidatorData(field);
    QRegExp regexp(regstr);                                                     /// подготовка рег. выражения из строки
    QValidator* validator = new QRegExpValidator(regexp, editor);               /// создание валидатора

    switch (curtype) {
    case QVariant::Date:{
        QDateEdit* dataedit = static_cast<QDateEdit*>(editor);
        Q_ASSERT(dataedit);
        dataedit->setDate(index.data().toDate());                               /// передать данные в редактор
        break;
    }
    case QVariant::String:{
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
        Q_ASSERT(lineedit);
        lineedit->setText(index.data().toString());                             /// передать данные в редактор
        lineedit->setValidator(validator);                                      /// привязка валидатора и поля ввода
        break;
    }
    case QVariant::Int:{
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
        Q_ASSERT(lineedit);
        lineedit->setText(index.data().toString());                             /// передать данные в редактор
        lineedit->setValidator(validator);                                      /// привязка валидатора и поля ввода
        break;
    }
    default: QStyledItemDelegate::setEditorData(editor, index);
    }

}

void RecordDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();
    QModelIndex atrivuteindex = index.model()->index(index.row(),0,index.parent());
    QString curatribut = atrivuteindex.data().toString();
    /// проверка является ли атрибут субтаблицей
    if(table->isForeignKey(index.row()))
    {
        DelegatButton *button = static_cast<DelegatButton*>(editor);
            model->setData(index, QVariant(button->text()));
        return ;
    }

    switch (curtype) {
    case QVariant::Date:{
        QDateEdit* dataedit = static_cast<QDateEdit*>(editor);                  /// класс widget преобразуется в реальный класс
        Q_ASSERT(dataedit);
        model->setData(index, dataedit->date());                                /// передать данные из редактора в модель
        break;
    }
    case QVariant::String:{
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
        Q_ASSERT(lineedit);
        model->setData(index, QVariant(lineedit->text()));
        break;
    }
    case QVariant::Int:{
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
        Q_ASSERT(lineedit);
        bool isnum;
        int num = lineedit->text().toInt(&isnum);
        if(isnum) model->setData(index, num);
        break;
    }
    default: QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void RecordDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    /*
        перерисовать элементы, отображающие мена полей таблицы,
        вместо реальных имен прорисовывать
    */
    /*if(index.column() == 0)
    {
           /* QStyleOptionViewItemV4 item(option);                                /// создается стиль прорисовки


            QStyledItemDelegate::paint(painter, option, index);
            //QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);/// прорисовка
       */
       /* QVariant val = table->getFieldAlterName(index.data(Qt::EditRole));      /// задается текст
        QStyleOptionViewItem reoption(option);
        reoption.text = val.toString();

        QStyledItemDelegate::paint(painter, reoption, QModelIndex());
        option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &reoption, painter);*/


    //return ;
    //}*/

    QStyledItemDelegate::paint(painter, option, index);                         /// стандартная прорисовка элемента


}

