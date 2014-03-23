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
#include "mytable.h"

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

    void setTitleText(QString text);                                            /// установить заголовок
    void setDisplayMode(bool titleVisible           = true,                     /// отображать заголовок
                        bool editbuttonpanelVisible = true,                     /// отображение панели с кнопками редактирования
                        bool headerMenuVisible      = true,                     /// отображение контектсного меняю для заголовков субтаблицы
                        bool dlgMovable             = true,                     /// перемещение окна
                        Qt::FocusPolicy tablefocuspolicy = Qt::WheelFocus
                        );
    //void setFieldDisplayedInEditor(QString field);                              /// установить атрибут, значение которого отображается в редакторе

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void newrowselected();

private slots:
    void on_tableView_clicked(const QModelIndex &index);

private:
    Ui::SubTableWidget *ui;

    QSpreadsheetHeaderView* header;                                             /// вьювер заголовока таблицы
    QStatusBar*         statusbar;
    QPoint              mpos;
    bool                movable;                                                /// перемещение окна

    MyTable*            table;
    //QString             displayedfield;                                         /// имя атрибута значение которого будет отображено в редакторе
};

#endif // SUBTABLEWIDGET_H
