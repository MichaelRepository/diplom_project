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

void RecordDelegate::setTableAttributeList(QMap<QString, SubTableWidget *> *list)
{
    tableattributelist = list;
}

QWidget *RecordDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определяет тип данных
    QModelIndex atributeindex = index.model()->index(index.row(),0);
    QString curatribut = atributeindex.data().toString();

    /// проверка является ли атрибут субтаблицей
    if(tableattributelist !=0 && tableattributelist->size() > 0 &&
       tableattributelist->contains(curatribut)){
            /// вернуть в качестве редактора диалог вывода субтаблицы
            DelegatButton *button = new DelegatButton(parent);
            SubTableWidget*subtabledlg = tableattributelist->value(curatribut);
            button->setSubTableDialog(subtabledlg);
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
    if(tableattributelist !=0 && tableattributelist->size() > 0 &&
       tableattributelist->contains(curatribut)){
        DelegatButton *button = static_cast<DelegatButton*>(editor);
        SubTableWidget *subtabledlg = tableattributelist->value(curatribut);
        QVariant value = subtabledlg->getDisplayedFieldValue();
        button->setText(value.toString());
        return ;
    }


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
       tableattributelist->contains(curatribut)){
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
    QStyledItemDelegate::paint(painter, option, index);                         /// стандартная прорисовка элемента

}

