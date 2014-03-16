#ifndef MESSDLG_H
#define MESSDLG_H

#include <QDialog>

namespace Ui {
class MessDlg;
}

class MessDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit MessDlg(QWidget *parent = 0);
    ~MessDlg();

    void setdata(QString titl, // установить
                 QString text,
                 QString info);
    void settitl(QString titl);// установить заголовок
    void settext(QString text);// установить текс
    void setinfo(QString info);// установить информацию
    
private slots:
    void on_OkButton_clicked();

private:
    Ui::MessDlg *ui;
};

#endif // MESSDLG_H
