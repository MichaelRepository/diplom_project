#include "recorddelegate.h"

RecordDelegate::RecordDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{

}

RecordDelegate::~RecordDelegate()
{

}

void RecordDelegate::setRegStrList(QList<QString> *list)
{
    regexplist = list;
}

void RecordDelegate::setTableAttributeList(QMap<QString, SubTableDialog *> *list)
{
    tableattributelist = list;
}

QWidget *RecordDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определяет тип данных
    QModelIndex atrivuteindex = index.model()->index(index.row(),0,index.parent());
    QString curatribut = atrivuteindex.data().toString();
    /// проверка является ли атрибут субтаблицей
    if(tableattributelist !=0 && tableattributelist->size() > 0 &&
       tableattributelist->contains(curatribut)){
            /// вернуть в качечтсве редактора кнопку вызова диалога вывода субтаблицы
            DelegatButton* button = new DelegatButton(parent);
            SubTableDialog *subtabledlg = tableattributelist->value(curatribut);
            //button->setSubTableDialog(subtabledlg);
            button->setFixedSize(option.rect.width(),option.rect.height());
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
    /*case QVariant::Map:{                                                        /// редактор - таблица(диалог субтаблиц)
        QComboBox* combobox = new QComboBox(parent);
        combobox->setFixedSize(option.rect.width(),option.rect.height());
        QStringList list
        combobox->addItems(index.data().toStringList());
        /// получить название текущего списка (соответсвует названию атрибута)
        QModelIndex atrivuteindex = index.model()->index(index.row(),0,index.parent());
        QString  curlist = atrivuteindex.data().toString();
        QVariant suritem;
        if(comboboxcuritem->contains(curlist) && comboboxcuritem->size() != 0)
            suritem = comboboxcuritem->value(curlist);
        else{
            MessDlg dlg;
            dlg.setdata(tr("Системная ошибка"),
                        tr("Ошибка: данные для отображения отсутствуют"),
                        "Произошла ошибка при попытке отобразить текущее значение атрибута "+curlist);
            dlg.exec();
            return combobox;
        }
        combobox->setCurrentText(suritem.toString());
        return combobox;
    }*/
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);        /// выбор за QStyledItemDelegate
    }

}

void RecordDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определяет тип данны
    QModelIndex atrivuteindex = index.model()->index(index.row(),0,index.parent());
    QString curatribut = atrivuteindex.data().toString();
    /// проверка является ли атрибут субтаблицей
    if(tableattributelist !=0 && tableattributelist->size() > 0 &&
       tableattributelist->contains(curatribut)) return ;


    /// создание валидатора для текущего редактора
    QString regstr;
    if(regexplist !=0 && regexplist->size() > 0 &&                              /// проверка корректности списка рег выражений
       index.row() < regexplist->size())
        regstr = regexplist->at(index.row());
    else
        regstr = ".{0,255}";                                                    /// если некорректный список, позволить вводить любые символы

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
    /*case QVariant::StringList:{
        QComboBox* combobox = static_cast<QComboBox*>(editor);

        combobox->setCurrentIndex(0);
        break;
    }*/

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
    if(tableattributelist !=0 && tableattributelist->size() > 0 &&
       tableattributelist->contains(curatribut))return ;

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
    /*case QVariant::StringList:{
        //QComboBox* combobox = static_cast<QComboBox*>(editor);
        //combobox->addItems(index.data().toStringList());
        //combobox->setCurrentIndex(0);
        break;
    }*/
    default: QStyledItemDelegate::setModelData(editor, model, index);
    }
}

/*void RecordDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                          const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}*/

void RecordDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();
    QModelIndex atrivuteindex = index.model()->index(index.row(),0,index.parent());
    QString curatribut = atrivuteindex.data().toString();

    QStyledItemDelegate::paint(painter, option, index);                         /// стандартная прорисовка элемента


    /// проверка является ли атрибут субтаблицей
   /* if(tableattributelist !=0 && tableattributelist->size() > 0 &&
       tableattributelist->contains(curatribut)){
        QStyleOptionViewItemV4 item(option);                                    /// создается стиль прорисовки
            item.text = "субтаблица";                                                     /// задается текст


        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &item, painter);/// прорисовка
    }*/



    /*
    dataedit->setFixedSize(option.rect.width(),option.rect.height());
    calendar->setFixedWidth(option.rect.width());

    QPixmap pixmap(dataedit->size());
    //if (!QPixmapCache::find(cacheKey, &pixmap)) {
    dataedit->render(&pixmap);
    //QPixmapCache::insert(cacheKey, pixmap);
    //}
    painter->drawPixmap(dataedit->rect(), pixmap);*/
}

