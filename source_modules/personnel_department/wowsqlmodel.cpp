#include "wowsqlmodel.h"

WoWSqlModel::WoWSqlModel(QObject *parent) :
    QSqlQueryModel(parent)
{
    refresh();
}

Qt::ItemFlags WoWSqlModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlQueryModel::flags(index);
    //flags |= Qt::ItemIsEditable;
    // первый столбец не редактируемый
    //if (index.column() == 4)    flags |= Qt::ItemIsUserCheckable;
    return flags;
}

QVariant WoWSqlModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);
    switch (role) {
    case Qt::DisplayRole:                                                       // Данные для отображения
    case Qt::EditRole:                                                          // данные для редактирования
        if (index.column() == 0)
            return value.toString().prepend(tr("#"));
        else if (index.column() == 5 && role == Qt::DisplayRole)                // отображение даты
            return value.toDate().toString("dd.MM.yyyy");
        else if (index.column() == 4)
            return value.toInt() ? tr("Муж") : tr("Жен");                       // отображение пола
        else
            return value;
    case Qt::TextColorRole:                                                     // Цвет текста
        if(index.column() == 1) return qVariantFromValue(QColor(Qt::blue));
        else
            return value;
    case Qt::TextAlignmentRole:                                                 // Выравнивание
        if(index.column() == 0)
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        else
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::FontRole:                                                          // Шрифт
        /*if(index.column() == 1) {
        QFont font = QFont("Helvetica", 10, QFont::Bold);
        return qVariantFromValue(font);
        }else*/
        return value;
    case Qt::BackgroundColorRole: {                                             // Цвет фона
        /*int a = (index.row() % 2) ? 14 : 0;
        if(index.column() == 0)
        return qVariantFromValue(QColor(220,240-a,230-a));
        else if(index.column() == 4)
        return qVariantFromValue(QColor(200,220-a,255-a));
        else*/
        return value;
        }
    case Qt::CheckStateRole:                                                    // Галочка
        /*if (index.column() == 4)
            return (QSqlQueryModel::data(index).toInt() != 0) ?
            Qt::Checked : Qt::Unchecked;
        else*/
            return value;
    case Qt::SizeHintRole:                                                      // Размер ячейки
        return value;
    }
}

QVariant WoWSqlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant value = QSqlQueryModel::headerData(section, orientation, role);
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
        return tr("");

    return value;
}

bool WoWSqlModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;

   /* QModelIndex primaryKeyIndex = QSqlQueryModel::index(index.row(), 0);
    int id = QSqlQueryModel::data(primaryKeyIndex).toInt();

    //clear(); // Если надо полностью перерисовать таблицу.

    bool ok;
    QSqlQuery query;
    if (index.column() == 1) {
        query.prepare("update employee set name = ? where id = ?");
        query.addBindValue(value.toString());
        query.addBindValue(id);
    }else if(index.column() == 2) {
        query.prepare("update employee set born = ? where id = ?");
        query.addBindValue(value.toDate());
        query.addBindValue(id);
    }else if(index.column() == 3) {
        query.prepare("update employee set salary = ? where id = ?");
        query.addBindValue(value.toDouble());
        query.addBindValue(id);
    }else if(index.column() == 4) {
        query.prepare("update employee set married = ? where id = ?");
        query.addBindValue(value.toBool());
        query.addBindValue(id);
    }
    ok = query.exec();
    refresh();*/
}

void WoWSqlModel::queryChange()
{
    refresh();
}

void WoWSqlModel::refresh()
{
    /** запрос данных **/

    /** подготовка заголовков таблицы **/
    setHeaderData(0, Qt::Horizontal, tr("Номер"));
    setHeaderData(1, Qt::Horizontal, tr("Фамилия"));
    setHeaderData(2, Qt::Horizontal, tr("Имя"));
    setHeaderData(3, Qt::Horizontal, tr("Отчество"));
    setHeaderData(4, Qt::Horizontal, tr("Пол"));
    setHeaderData(5, Qt::Horizontal, tr("Дата рождения"));
    setHeaderData(6, Qt::Horizontal, tr("Место рождения"));
    setHeaderData(7, Qt::Horizontal, tr("Тип субъекта"));
}
