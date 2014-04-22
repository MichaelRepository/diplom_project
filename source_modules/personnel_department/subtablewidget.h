#ifndef SUBTABLEWIDGET_H
#define SUBTABLEWIDGET_H

#include <QWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QInputEvent>
#include <QSql>

#include <QList>
#include <QPair>

#include "dbmessdlg.h"
#include "QSpreadsheetHeaderView.h"
#include "MyTable/mytable.h"
#include <QAbstractItemModel>

/*enum SubTable{CITIZENSHIP, RESIDENCE, PASSPORT, REPRESENT,
              EDUCATION, PRIVILEGES,  MOREINF};*/

namespace Ui {
class SubTableWidget;
}

class SubTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SubTableWidget(QWidget *parent = 0);
    ~SubTableWidget();

    void setTableModel(QAbstractItemModel* model);

    void setTitleText(QString text);                                            /// установить заголовок
    void setDisplayMode(bool titleVisible     = true,
                        bool menuPanelVisible = true,
                        bool dlgMovable       = true,
                        Qt::FocusPolicy tablefocuspolicy = Qt::NoFocus);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setRow(int row);
    int  getSelectedRowIndex();

signals:
    void newrowselected(int);
    void editButtonClicked();
    void removeButtonClicked();
    void addButtonClicked();

private slots:
    void on_tableView_clicked(const QModelIndex &index);
    void on_Edit_button1_clicked();
    void on_Add_button1_clicked();
    void on_Delete_button1_clicked();


private:
    Ui::SubTableWidget *ui;

    QSpreadsheetHeaderView* header;                                             /// вьювер заголовока таблицы
    QHeaderView*            standartheader;
    QStatusBar*             statusbar;
    QPoint                  mpos;
    bool                    movable;                                            /// перемещение окна
    QAbstractItemModel*     tablemodel;
};

#endif // SUBTABLEWIDGET_H
