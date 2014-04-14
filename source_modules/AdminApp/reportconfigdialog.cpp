#include "reportconfigdialog.h"
#include "ui_reportconfigdialog.h"

ReportConfigDialog::ReportConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportConfigDialog)
{
    ui->setupUi(this);

    QIcon icon = windowIcon();
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    setWindowIcon(icon);

    QRegExp regexp("(\\w|\\s)+");
    QValidator* validator = new QRegExpValidator(regexp,this);
    ui->NameEdit->setValidator(validator);

    mode = ADDMODE;

    ui->comboBox->setToolTip    ("Укажите группу пользователей для которых создается шаблон отчета.");
    ui->NameEdit->setToolTip    ("Укажите название отчета.");
    ui->InfoEdit->setToolTip    ("Область для создания описания к отчету.");
    ui->FilePathEdit->setToolTip("Поле для ввода пути к документу шаблона. (Формат .odt)");
    ui->BrowsBt->setToolTip     ("Вызвать диалоговое окно открытия/сохраения документа.");
}

ReportConfigDialog::~ReportConfigDialog()
{
    delete ui;
}

QString ReportConfigDialog::repName()
{
    return ui->NameEdit->text();
}

QString ReportConfigDialog::repInfo()
{
    return ui->InfoEdit->toPlainText();
}

QString ReportConfigDialog::repFile()
{
    return ui->FilePathEdit->text();
}

int ReportConfigDialog::userGroup()
{
    return ui->comboBox->currentData().toInt();
}

void ReportConfigDialog::setData(QString repname, QString repinfo, int usergroupid,
                                 dialogMode dlgmode)
{
    ui->FilePathEdit->clear();

    ui->NameEdit->setText(repname);
    ui->InfoEdit->setText(repinfo);
    QAbstractItemModel* model = ui->comboBox->model();
    for(int i = 0; i < model->rowCount(); ++i)
    {
        QModelIndex index = model->index(i,0);
        if(model->data(index, Qt::UserRole).toInt() == usergroupid)
        {
            ui->comboBox->setCurrentIndex(i);
            break;
        }
    }
    mode = dlgmode;
}

void ReportConfigDialog::setGroupList(const QMap<int, QString> &grouplist)
{
    QMap<int, QString>::const_iterator itr;
    QIcon ico(":/svg/group-icon.svg");
    for(itr = grouplist.begin(); itr != grouplist.end(); ++itr)
    {
        ui->comboBox->addItem(ico, itr.value(), QVariant(itr.key()) );
    }
}

void ReportConfigDialog::on_BrowsBt_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open ODT file",
                                                    "",
                                                    "Open document (*.odt)"
                                                    );
    ui->FilePathEdit->setText(filename);
    if(ui->NameEdit->text().size() == 0)
    {
        QUrl url = QUrl::fromLocalFile(filename);
        filename = url.fileName();
        filename.remove(".odt", Qt::CaseInsensitive);
        ui->NameEdit->setText(filename);
    }
}

void ReportConfigDialog::on_CancelBt_clicked()
{
    done(0);
}

void ReportConfigDialog::on_YesBt_clicked()
{

    if(mode != EDITMODE ||
      (mode == EDITMODE && ui->FilePathEdit->text().size() != 0))
    {
        QFileInfo inf(ui->FilePathEdit->text());
        if(!inf.isFile())
        {
            QMessageBox dlg;
            dlg.setWindowIcon(this->windowIcon());
            dlg.setWindowTitle("Не задан путь к файлу");
            dlg.setText("Укажите путь к файлу формата .odt");
            dlg.addButton(QMessageBox::Ok);
            dlg.setButtonText(QMessageBox::Ok,"Ясно");
            dlg.setIcon(QMessageBox::Warning);
            dlg.exec();

            return ;
        }
    }

    if(ui->NameEdit->text().size() == 0)
    {
        QMessageBox dlg;
        dlg.setWindowIcon(this->windowIcon());
        dlg.setWindowTitle("Не задано имя");
        dlg.setText("Укажите название отчета");
        dlg.addButton(QMessageBox::Ok);
        dlg.setButtonText(QMessageBox::Ok,"Ясно");
        dlg.setIcon(QMessageBox::Warning);
        dlg.exec();

        return ;
    }

    done(1);
}

void ReportConfigDialog::closeEvent()
{
    done(0);
}

void ReportConfigDialog::on_FilePathEdit_textChanged(const QString &)
{
    QFileInfo inf(ui->FilePathEdit->text());
    QFont font = ui->FilePathEdit->font();

    if(inf.isFile())
        ui->FilePathEdit->setStyleSheet("color:#00B515;");
    else
        ui->FilePathEdit->setStyleSheet("color:#FF0000;");
}
