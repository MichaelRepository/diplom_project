#include "recorddelegate.h"

RecordDelegate::RecordDelegate(QString connectname, QObject * parent):
    QStyledItemDelegate(parent)
{
    connectionname = connectname;  
    querylist = new QMap<int, QSqlQueryModel*>();
}

RecordDelegate::~RecordDelegate()
{
    if(!querylist->isEmpty() )
    {
        QList<QSqlQueryModel*> list = querylist->values();
        qDeleteAll(list);
    }
    delete querylist;
}

QWidget *RecordDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    int indexrow = index.row();

    /// проверка является ли поле внешним ключем (ссылающимся на внешнюю таблицу)
    /// вернуть в качестве редактора - диалог вывода субтаблицы
    if(!index.data(Qt::UserRole+2).toString().isEmpty() )
    {
        QSqlQueryModel* model;
        if(querylist->contains(indexrow) ) // если ранее данные были получены с бд
        {
            model  = querylist->value(indexrow, 0);
            if(model->lastError().isValid() ){qDebug() << model->lastError(); return 0;}
        }
        else // не были получены -> получить и сохранить
        {
            QSqlDatabase db = QSqlDatabase::database(connectionname);
            if(!db.open()) return 0;
            model = new QSqlQueryModel();
            model->setQuery(index.data(Qt::UserRole+2).toString(), db);
            QStringList alternames = index.data(Qt::UserRole+3).toStringList();
            QStringList::iterator itr;
            /// переименовать столбцы модели
            int j = 0;
            for(itr = alternames.begin(); itr != alternames.end(); ++itr)
            {
                model->setHeaderData(j, Qt::Horizontal,QVariant(*itr));
                ++j;
            }
            querylist->insert(indexrow, model);
            if(model->lastError().isValid() ) return 0;
        }
        DelegatButton *button1 = new DelegatButton(index, model, parent);
        return button1;
    }

    /// анализ типа данных (по стандарту СУБД)
    QString realtype = index.data(Qt::UserRole).toString();
    if(realtype == "ENUM" || realtype == "TINYINT")
    {
        QComboBox* combo = new QComboBox(parent);
        combo->addItems(index.data(Qt::EditRole).toStringList() );
        combo->setCurrentText(index.data(Qt::DisplayRole).toString() );
        combo->resize(option.rect.size());
        return combo;
    }
    else if(realtype == "DATE")
    {
        QDateEdit* dataedit = new QDateEdit(parent);                            /// редактор - календарь
        dataedit->setCalendarPopup(true);
        dataedit->resize(option.rect.size() );
        return dataedit;
    }
    else
    {
        QLineEdit* lineedit = new QLineEdit(parent);                            /// редактор - поле ввода
        lineedit->resize(option.rect.size() );
        return lineedit;
    }

}

void RecordDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
    if(!index.data(Qt::UserRole+2).toString().isEmpty() ) return ;
    QString realtype = index.data(Qt::UserRole).toString();
    if(realtype.isEmpty() ){ qDebug() << "realtype = NULL"; return ;}
    if(realtype == "ENUM" || realtype == "TINYINT") return ;

    /// создание валидатора для текущего редактора
    QString regstr = index.data(Qt::UserRole+1).toString();
    QRegExp regexp(regstr);                                                     /// подготовка рег. выражения из строки
    QValidator* validator = new QRegExpValidator(regexp, editor);               /// создание валидатора

    /// анализ типа данных (по стандарту СУБД)
    if(realtype == "ENUM" || realtype == "TINYINT")
    {
        QComboBox* combo = static_cast<QComboBox*>(editor);
        Q_ASSERT(combo);
        combo->setCurrentText(index.data(Qt::DisplayRole).toString() );
        return ;
    }
    else if(realtype == "DATE")
    {
        QDateEdit* dataedit = static_cast<QDateEdit*>(editor);                  /// редактор - календарь
        Q_ASSERT(dataedit);
        dataedit->setDate(index.data(Qt::DisplayRole).toDate() );
        return ;
    }
    else
    {
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);                  /// редактор - поле ввода
        Q_ASSERT(lineedit);
        lineedit->setText(index.data(Qt::DisplayRole).toString() );
        lineedit->setValidator(validator);
        return ;
    }

}

void RecordDelegate::setModelData(QWidget *editor,
                                  QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QString realtype = index.data(Qt::UserRole).toString();
    if(!index.data(Qt::UserRole+2).toString().isEmpty() )
    {
        DelegatButton *button = static_cast<DelegatButton*>(editor);
        model->setData(index, button->realValue(), Qt::EditRole);
        return ;
    }
    else if(realtype == "ENUM" || realtype == "TINYINT")
    {
        QComboBox* combo = static_cast<QComboBox*> (editor);
        Q_ASSERT(combo);
        model->setData(index, combo->currentText(), Qt::EditRole );
        return ;
    }
    else if(realtype == "DATE")
    {
        QDateEdit* dataedit = static_cast<QDateEdit*>(editor);                  /// редактор - календарь
        Q_ASSERT(dataedit);
        model->setData(index, dataedit->date(), Qt::EditRole );
        return ;
    }
    else
    {
        QLineEdit* lineedit = static_cast<QLineEdit*>(editor);                  /// редактор - поле ввода
        Q_ASSERT(lineedit);
        model->setData(index, lineedit->text(), Qt::EditRole );
        return ;
    }

}

void RecordDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);


}

