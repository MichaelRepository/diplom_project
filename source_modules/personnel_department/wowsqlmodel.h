#ifndef WOWSQLMODEL_H
#define WOWSQLMODEL_H

#include <QtSql>
#include <QSqlQueryModel>
#include <QTableView>
#include <QDate>

class WoWSqlModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit WoWSqlModel(QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    
signals:
    
public slots:

private:
    void queryChange();
    void refresh();
    
};

#endif // WOWSQLMODEL_H
