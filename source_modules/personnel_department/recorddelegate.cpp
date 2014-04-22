#include "recorddelegate.h"

RecordDelegate::RecordDelegate(QString connectname, MySqlRecord* record, QObject * parent):
    QStyledItemDelegate(parent)
{
    connectionname = connectname;
    recorddata     = record;
}

RecordDelegate::~RecordDelegate()
{

}

void RecordDelegate::initDelegate()
{
    QSqlDatabase db = QSqlDatabase::database(connectionname);
    if(!db.open())
    {
        dbMessDlg dlg;
        dlg.showdbmess(db.lastError());
        return ;
    }
    /// подготавливаются модели для отображения
    /// данных из слинкованных таблиц
    for(int i = 0; i < recorddata->count(); ++i)
    {
        if(recorddata->isForeign(i))
        {
            MyDataReference reference = recorddata->referenceDataOfField(i);
            QSqlQueryModel *query = new QSqlQueryModel(this);
            query->setQuery(reference.sqltext,db);
            if(query->lastError().isValid())
            {
                dbMessDlg dlg;
                dlg.showdbmess(db.lastError());
                return ;
            }
            QStringList::iterator itr;
            int j = 0;
            for(itr = reference.alternames.begin(); itr != reference.alternames.end(); ++itr)
            {
                query->setHeaderData(j,Qt::Horizontal,QVariant(*itr));
                ++j;
            }
            querylist.insert(i,query);
        }
    }
}

QWidget *RecordDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QVariant::Type curtype    = index.data().type();                            /// определяет тип данных

    int indexrow = index.row();

    /// проверка является ли поле внешним ключем (ссылающимся на внешнюю таблицу)
    if(recorddata->isForeign(indexrow) )
    {
        /// вернуть в качестве редактора - диалог вывода субтаблицы
        QSqlQueryModel* model  = querylist[indexrow];
        DelegatButton *button1 = new DelegatButton(indexrow, recorddata, model, parent);
        return button1;
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

    if(recorddata->isForeign(index.row()))
    {
        return ;
    }

    /// создание валидатора для текущего редактора
    QString regstr = recorddata->validatorOfField(index.row());
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
    /// проверка является ли поле внешним ключем (ссылающимся на внешнюю таблицу)
    if(recorddata->isForeign(index.row()) )
    {
        /*DelegatButton *button = static_cast<DelegatButton*>(editor);
        recorddata->setValue(index.row(),fieldata.value());
        if(fieldata.alterField().size() > 0)
            recorddata->setAlterData(index.row(),fieldata.alterData());*/
        return ;
    }

    switch (curtype) {
    case QVariant::Date:{
        QDateEdit* dataedit = static_cast<QDateEdit*>(editor);                  /// класс widget преобразуется в реальный класс
        Q_ASSERT(dataedit);
        recorddata->setValue(index.row(), QVariant(dataedit->date() ) );
        //model->setRecordData(index, dataedit->date());                          /// передать данные из редактора в модель
        break;
    }
    case QVariant::String:{
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
        Q_ASSERT(lineedit);
        recorddata->setValue(index.row(), QVariant(lineedit->text() ) );
        //model->setRecordData(index, QVariant(lineedit->text()));
        break;
    }
    case QVariant::Int:{
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
        Q_ASSERT(lineedit);
        bool isnum;
        int num = lineedit->text().toInt(&isnum);
        if(isnum) recorddata->setValue(index.row(), QVariant(num))/*model->setRecordData(index, num)*/;
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

