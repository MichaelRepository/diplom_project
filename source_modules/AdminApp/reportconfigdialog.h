#ifndef REPORTCONFIGDIALOG_H
#define REPORTCONFIGDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

enum dialogMode {ADDMODE, EDITMODE};

namespace Ui {
class ReportConfigDialog;
}

class ReportConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportConfigDialog(QWidget *parent = 0);
    ~ReportConfigDialog();

    QString repName();
    QString repInfo();
    QString repFile();
    int     userGroup();

    void setGroupList(const QMap<int, QString> &grouplist);
    void setData(QString repname, QString repinfo, int usergroupid,
                 dialogMode dlgmode = EDITMODE);

private slots:
    void on_BrowsBt_clicked();
    void on_CancelBt_clicked();
    void on_YesBt_clicked();

    void closeEvent();

    void on_FilePathEdit_textChanged(const QString &arg1);

private:
    Ui::ReportConfigDialog *ui;

    dialogMode mode;
};

#endif // REPORTCONFIGDIALOG_H
