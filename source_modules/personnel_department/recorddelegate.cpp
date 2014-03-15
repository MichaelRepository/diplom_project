#include "recorddelegate.h"

RecordDelegate::RecordDelegate()
{

}

RecordDelegate::~RecordDelegate()
{

}

void RecordDelegate::setRegStrList(QList<QString> *list)
{
    regexplist = list;
}

QWidget *RecordDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определяет тип данных

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
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);        /// выбор отсается за QStyledItemDelegate
    }

}

void RecordDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
    QVariant::Type curtype = index.data().type();                               /// определить тип данных

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

                                                                                /// создание валидатора для поля ввода
        QRegExp regexp(regexplist->at(index.row()));                            /// подготовка рег. выражения из строки
        QValidator* validator = new QRegExpValidator(regexp, lineedit);         /// создание валидатора
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
        model->setData(index, lineedit->text());
        break;
    }
    default: QStyledItemDelegate::setModelData(editor, model, index);
    }
}

/*void RecordDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    RecordDelegate::paint(painter,option,index);
    /*
    dataedit->setFixedSize(option.rect.width(),option.rect.height());
    calendar->setFixedWidth(option.rect.width());

    QPixmap pixmap(dataedit->size());
    //if (!QPixmapCache::find(cacheKey, &pixmap)) {
    dataedit->render(&pixmap);
    //QPixmapCache::insert(cacheKey, pixmap);
    //}
    painter->drawPixmap(dataedit->rect(), pixmap);
}*/

