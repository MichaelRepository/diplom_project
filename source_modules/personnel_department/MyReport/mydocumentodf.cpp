#include "mydocumentodf.h"

MyDocumentODF::MyDocumentODF(QObject *parent) :
    QObject(parent)
{
    error = MYDOC_NOERROR;
    contentfile = 0;
    stylesfile  = 0;

    /// создание окна ожидания
    catdlg = new QDialog();
    QGridLayout *layout = new QGridLayout(catdlg);
    QLabel *labelmovie  = new QLabel(catdlg);
    QLabel *label       = new QLabel(catdlg);
    catmovie            = new QMovie("://cat.gif");

    catdlg->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(labelmovie);
    layout->addWidget(label);

    label->setText("Формирование отчета...");
    label->setStyleSheet("background-color: #3C3C3C; color:#fff; font-size:15px;");
    label->setAlignment(Qt::AlignCenter);
    label->resize(20,20);
    labelmovie->setMovie(catmovie);
    catdlg->setWindowFlags(Qt::SplashScreen);

}

MyDocumentODF::~MyDocumentODF()
{
    catmovie->deleteLater();
    catdlg->deleteLater();
}

bool MyDocumentODF::readDocumentData(QByteArray &data)
{
    bool result = true;
    error = MYDOC_NOERROR;

    catmovie->start();
    catdlg->show();

    /// основная логика по разбору шаблона документа
    result = readAllZippedFile(data);                                           /// получения массивов данных каждого файла архива
    if(result) result = parsDocContent();                                       /// разбор структуры двух основных файлов content и styles

    /// поиск всех заменяемых элементов (полей-переменных)
    /// формируются variables - списки переменных внутри каждой из структур
    findVariableSet(contentstruct.bodyroot,    &contentstruct.variables);
    findVariableSet(stylesstruct.master_page,  &stylesstruct.variables);

    /// замена переменных в content
    /// обработка списка variables
    if(result && contentstruct.variables.size() != 0 )
    {
        QList<QDomElement>::iterator itr;
        for(itr = contentstruct.variables.begin(); itr != contentstruct.variables.end(); ++itr)
        {
            /// анализ объекта подстановки, являющегося значением поля-переменной
            result &= replaceVariables(*itr, &contentstruct.dom);
            if(!result) break;
        }
    }
    /// замена переменных в styles
    if(result && stylesstruct.variables.size() != 0 )
    {
        QList<QDomElement>::iterator itr;
        for(itr = stylesstruct.variables.begin(); itr != stylesstruct.variables.end(); ++itr)
        {
            /// анализ объекта подстановки, являющегося значением поля-переменной
            result &= replaceVariables(*itr, &stylesstruct.dom);
            if(!result) break;
        }
    }

    /// получение обработанных данных для последующего сохранения в файл
    if(result)
    {
        contentfile->data = contentstruct.dom.toByteArray();
        stylesfile->data  = stylesstruct.dom.toByteArray();
    }

    catmovie->stop();
    return result;
}

bool MyDocumentODF::saveDocumentCopy(QString newfile)
{
    return writeZippedFiles(newfile);
}

bool MyDocumentODF::createDocumentFrom(QSqlQuery &querydata,
                                       const QStringList &fieldsname,
                                       const QList<int> &fields)
{
    bool result = true;
    error = MYDOC_NOERROR;

    catmovie->start();
    catdlg->show();

    QFile templatedoc("://template_.odt");
    if(!templatedoc.open(QIODevice::ReadOnly) ) return false;
    QByteArray data = templatedoc.readAll();
    templatedoc.close();

   // _querydata = querydata;
   // _fields    = fields;

    result = readAllZippedFile(data);                                           /// получения массивов данных каждого файла архива
    if(result) result = parsDocContent();

    createTable(contentstruct, querydata, fields, fieldsname); // создать таблицу

    /// получение обработанных данных для последующего сохранения в файл
    if(result)
    {
        contentfile->data = contentstruct.dom.toByteArray();
        stylesfile->data  = stylesstruct.dom.toByteArray();
    }

    catmovie->stop();
    return result;

}

void MyDocumentODF::setConnectionName(QString connect)
{
    connection = connect;
}

bool MyDocumentODF::writeZippedFiles(QString file)
{
    QuaZip zip(file);
    zip.open(QuaZip::mdCreate );
    QList<MyFileFromZip>::const_iterator itr;
    for(itr = files.begin(); itr != files.end(); ++itr)
    {
        QuaZipFile zipfile(&zip);
        QuaZipNewInfo info((*itr).name);
        if(!zipfile.open(QIODevice::WriteOnly, info) )
        {
            qDebug() << zipfile.errorString();
            error = MYDOC_ZIPFILEERROR;
            return false;
        }

        zipfile.write((*itr).data);
        zipfile.close();

    }
    zip.close();
}

void MyDocumentODF::findVariableSet(const QDomElement &element, QList<QDomElement>* variables)
{
    QDomNode node = element.firstChild();
    while (!node.isNull())
    {
      if (node.isElement())
      {
          if (node.toElement().tagName() == "text:variable-set")
              variables->append(node.toElement());
          else
              findVariableSet(node.toElement(),variables);
      }
      node = node.nextSibling();
    }
}

bool MyDocumentODF::replaceVariables(QDomElement &element, QDomDocument* dom)
{
    /**  замена объекта - переменной данными
     * тип переменной определяет расположение значения переменной
     * string - содержит значение внутри value
     * float  - содержит значение в параметре formula
    **/
    QString value_type = element.attribute("office:value-type").toUpper();      /// определить тип переменной
    QString value = "";
    if(value_type == "STRING") value   = element.text();
    if(value_type == "FLOAT")  value   = element.attribute("text:formula");
    value.remove("ooow:",Qt::CaseInsensitive);

    MyRepObjectData objectdata = parsVariableValue(value);                      /// пропарсить значение поля-перменной
    if(!objectdata.isValid()) return false;

    QDomNode replaceablenode = element;                                         /// заменяемая нода

    ///  замена в соответствии с типом объекта подставновки
    switch(objectdata.type){
    case MYREP_NOTYPE:
        error = MYDOC_OBJECTSTRUCTERROR;
        return false;
        break;
    case MYREP_TABLE:{
        /// проверка, вложено поле в таблицу (контрольный элемент расположен на два уровня выше поля-переменной)
        QDomNode control = element.parentNode();
        QString tagname;
        /// получить родительскую ячейку для текущего поля-переменной
        while( tagname != "table:table-cell" && tagname != "office:body")
        {
            control = control.parentNode();
            tagname = control.toElement().tagName();
        }
        /// ячейка отсутсвует (поиск остановился на элементе body)
        if(control.toElement().tagName() != "table:table-cell")
        {
            /// если не вложено - сообщить об ошибке
            error = MYDOC_OBJECTSTRUCTERROR;
            return false;
            /* возможно стоит изменить алгоритм:
             * если поле-переменная находится вне таблицы, необходимо
             * создать новую таблицу.
             * это потребует дополнительных манипуляций по созданию
             * новых узлов в DOM файла content
            */
        }
        if(objectdata.valueisquery ) /// отчет запросил данные из БД
        {
            /// получить данные из БД
            QSqlDatabase db = QSqlDatabase::database(connection);
            QSqlQuery query(db);
            if(!query.exec(objectdata.value) ) { error = MYDOC_OBJECTSTRUCTERROR; return false;}
            //if(query.size() == 0)              { error = DATANOTFOUND; return false;}
            if(query.size() == 0)
            {
                QDomNode newelement = control.firstChildElement().cloneNode(false);
                QDomNode removed    = control.replaceChild(newelement, control.firstChildElement());
                removed.clear();
                break;
            }
            /// разместить данные в таблице отчета
            QDomNode firstrow   = control.parentNode();
            QDomNode table      = firstrow.parentNode();

            int rowindex = 0;
            while (query.next())
            {
                QDomNode    newrow;
                if(rowindex == 0) newrow = firstrow;
                else              newrow = firstrow.cloneNode();
                QDomElement cell = newrow.firstChildElement();

                int colindex = 0;
                while(!cell.isNull())                                               /// обойти все ячеки текущей строки
                {
                    QDomElement cellcontent = cell.firstChildElement();             /// получить первый элемент в ячейке
                    QDomElement newelement  = dom->createElement("text:span");      /// создать новый элемент p
                    /// создать текстовый элемент со значением из БД
                    QDomText    textelement;
                    QString value = "";
                    if(colindex < query.record().count()) value = query.value(colindex).toString();

                    textelement = dom->createTextNode(value);

                    newelement.appendChild(textelement);                            /// добавить элементу - p текстовый элемент
                    /// заменить содержимое ячейки
                    QDomNode newcellcontent = cellcontent.cloneNode(false);         /// новое содержимое ячейки - это чистая (без вложений) копия первого чилд узла
                    QDomNode removednode    = cell.replaceChild(newcellcontent,
                                                                cellcontent);
                    removednode.clear();
                    newcellcontent.appendChild(newelement);

                    cell = cell.nextSiblingElement();
                    ++colindex;
                }
                table.appendChild(newrow);
                ++rowindex;
            }
        }
        else
        {
            /// если объект не содержит запроса к БД
            error = MYDOC_OBJECTSTRUCTERROR;
            return false;
        }
        break;
    }
    case MYREP_LIST:{
        if(!objectdata.valueisquery)
        {
            /// если объект не содержит запроса к БД
            error = MYDOC_OBJECTSTRUCTERROR;
            return false;
        }
        /// получить данные из БД
        QSqlDatabase db = QSqlDatabase::database(connection);
        QSqlQuery query(db);
        if(!query.exec(objectdata.value) ) { error = MYDOC_OBJECTSTRUCTERROR; return false;}
        //if(query.size() == 0)              { error = DATANOTFOUND; return false;}
        if(query.size() == 0)
        {
            QDomNode parentnode = replaceablenode.parentNode();
            QDomNode newelement = dom->createElement("text:span");
            QDomNode removed    = parentnode.replaceChild(newelement, replaceablenode);
            removed.clear();
            break;
        }

        QDomNode parentnode    = replaceablenode.parentNode();
        QDomNode parentforlist = dom->createElement("text:span");/// родитель для списка
        /// заменить и удалить поле-ссылку
        QDomNode removednode = parentnode.replaceChild(parentforlist, replaceablenode);
        removednode.clear();
        int rowindex = 0;
        while(query.next())
        {
            QDomNode linebreak   = dom->createElement("text:line-break");
            QDomText textelement = dom->createTextNode(query.value(0).toString());
            parentforlist.appendChild(textelement);
            parentforlist.appendChild(linebreak);
            ++rowindex;
        }
        break;
    }
    case MYREP_LINE:{
        QDomNode parentnode = replaceablenode.parentNode();
        QDomNode newline = dom->createElement("text:span");/// новый элемент
        /// заменить и удалить поле-ссылку
        QDomNode removednode = parentnode.replaceChild(newline,replaceablenode);
        removednode.clear();

        QString textvalue;
        if(objectdata.valueisquery)
        {
            QSqlDatabase db = QSqlDatabase::database(connection);
            QSqlQuery query(db);
            if(!query.exec(objectdata.value) ) { error = MYDOC_OBJECTSTRUCTERROR; return false;}
            if(query.size() != 0)
            {
                query.first();
                textvalue = query.value(0).toString();
            }
        }
        else
            textvalue = objectdata.value;
        QDomNode newtextnode = dom->createTextNode(textvalue);                  /// новый элемент
        newline.appendChild(newtextnode);
        break;
    }
    }

    return true;
}

MyRepObjectData MyDocumentODF::parsVariableValue(QString value)
{
    QString         objtypetext;                                                /// тип замещаемого объекта
    MyRepObjectData resobjdata;

    QList<MyReplaceableField> resultlist;                                       /// локальный список переменных

    /// выделить тип объекта и строку запроса
    QRegExp rx("^\\s*\\b(TABLE|LIST|LINE|VARIABLE)\\b\\s*=\\s*(.*)",Qt::CaseInsensitive);
    if(!rx.exactMatch(value) ){error = MYDOC_OBJECTSTRUCTERROR; return MyRepObjectData();}

    objtypetext        = rx.cap(1).simplified();                                /// тип объекта
    resobjdata.value   = rx.cap(2).simplified();                                /// значение объекта

    if(objtypetext.toUpper() == "TABLE") resobjdata.type = MYREP_TABLE;
    if(objtypetext.toUpper() == "LIST")  resobjdata.type = MYREP_LIST;
    if(objtypetext.toUpper() == "LINE")  resobjdata.type = MYREP_LINE;

    /// валидация строки запроса
    QRegExp rxv("\\b(CREATE|ALTER|DROP|INSERT|UPDATE|DELETE|GRANT|REVOKE|DENY)\\b",
                Qt::CaseInsensitive);
    if(rxv.exactMatch(resobjdata.value) ) {error = MYDOC_OBJECTSTRUCTERROR; return MyRepObjectData();}

    /// проверка, является значение объекта подстановки запросом
    rxv.setPattern("\\bSELECT\\b");
    resobjdata.valueisquery = (rxv.indexIn(resobjdata.value,0) != -1);

    /// выделить все элементы - переменные
    QRegExp rx1("\\[\\?([\\w|\\s]+)(?::\\s*(string|num|date|(?:(enum)?(?:\\{((.+))\\}))))?\\s*\\]",
                Qt::CaseInsensitive);
    int pos = 0;
    while ((pos = rx1.indexIn(resobjdata.value,pos)) != -1)
    {
        MyReplaceableField robject;
        robject.namevar  = rx1.cap(1).simplified();
        /// проверка нет ли уже такого объекта
        bool inStock = false;
        QList<MyReplaceableField>::const_iterator itr;
        for(itr = globalrepfiellist.begin(); itr != globalrepfiellist.end(); ++itr)
        {
            if((*itr).namevar == robject.namevar)
            {
                robject = *itr;
                inStock = true;
            }
        }

        robject.pos  = pos;
        robject.leng = rx1.matchedLength();

        if(!inStock)
        {
            QString texttype = rx1.cap(2).simplified().toUpper();
            if(rx1.cap(3).toUpper().simplified() == "ENUM")                         /// при enum происходин смещение с cap(2) в cap(3)
            {
                /// переписать тип данных на реальный
                texttype = rx1.cap(3).toUpper().simplified();
                /// получить список вариантов для перечисления
                robject.enumlist = rx1.cap(5).split(QRegExp("\\s*,\\s*"),
                                                    QString::SkipEmptyParts);
            }

            if(texttype == "STRING") robject.typevar = MYREP_STRING;
            if(texttype == "NUM")    robject.typevar = MYREP_NUM;
            if(texttype == "DATE")   robject.typevar = MYREP_DATE;
            if(texttype == "ENUM")   robject.typevar = MYREP_ENUM;
        }
        else
            robject.typevar = MYREP_FIELD_NULL;                                 /// если есть такое поле, то сделать текущее не читабельным

        resultlist.append(robject);

        pos +=rx1.pos(0)+rx1.matchedLength();
    }

    if(resultlist.size() == 0)   return resobjdata;                             /// нет замещаемых объектов

    /// подготовить диалоговое окно
    QDialog mydlg;
    mydlg.setWindowTitle("Введите данные");
    QGridLayout *layout = new QGridLayout(&mydlg);
    mydlg.setLayout(layout);

    int addwidgets = 0;

    QLabel *mainlabel = new QLabel(&mydlg);
    mainlabel->setText("Необходимо указать следующие данные");
    mainlabel->setMinimumHeight(25);
    layout->addWidget(mainlabel,0,0);

    QList<MyReplaceableField>::iterator itr;
    int row = 1;
    /// разместить в диалоге виджеты в соответсвии с типом данных
    for(itr = resultlist.begin(); itr != resultlist.end(); ++itr)
    {
        if((*itr ).typevar != MYREP_FIELD_NULL)                                   /// если объект читабелен
        {
            QLabel *label = new QLabel(&mydlg);
            label->setText((*itr).namevar);
            layout->addWidget(label,row,0);
            ++row;

            switch((*itr).typevar)
            {
            case MYREP_STRING:{
                QLineEdit *edit = new QLineEdit(&mydlg);
                QRegExpValidator *validator = new QRegExpValidator(QRegExp(".{0,255}"),edit);
                layout->addWidget(edit,row,0);
                edit->setValidator(validator);
                (*itr).widget = edit;
                ++addwidgets;
                break;
            }
            case MYREP_NUM:{
                QSpinBox *spin = new QSpinBox(&mydlg);
                layout->addWidget(spin,row,0);
                (*itr).widget = spin;
                ++addwidgets;
                break;
            }
            case MYREP_DATE:{
                QDateEdit *dateed = new QDateEdit(&mydlg);
                layout->addWidget(dateed,row,0);
                (*itr).widget = dateed;
                ++addwidgets;
                break;
            }
            case MYREP_ENUM:{
                QComboBox *combo = new QComboBox(&mydlg);
                layout->addWidget(combo,row,0);
                (*itr).widget = combo;
                combo->addItems((*itr).enumlist);
                ++addwidgets;
                break;
            }
            }
        }
        ++row;
    }
    layout->setRowStretch(row,1);

    QDialogButtonBox *buttonbox = new QDialogButtonBox(&mydlg);
    QPushButton *bt1 = buttonbox->addButton(QDialogButtonBox::Ok);
    QPushButton *bt2 = buttonbox->addButton(QDialogButtonBox::Cancel);
    layout->addWidget(buttonbox,++row,0);
    bt1->setText("Применить");
    bt2->setText("Отмена");

    connect(buttonbox, &QDialogButtonBox::accepted, &mydlg, &QDialog::accept);
    connect(buttonbox, &QDialogButtonBox::rejected, &mydlg, &QDialog::reject);

    mydlg.setMaximumWidth(250);
    mydlg.setMinimumWidth(250);
    mydlg.setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    mydlg.setWindowFlags(Qt::WindowContextHelpButtonHint);
    if(addwidgets > 0)
    {
        /// действее отменено пользователем (MYDOC_NOERROR - ошибки нету!)
        if(!mydlg.exec()) {error = MYDOC_NOERROR; return MyRepObjectData();}
    }

    /// обход списка переменных в обратном порядке и их подмена значениями
    itr = resultlist.end();
    do
    {
        --itr;
        QString value="";
        if((*itr ).typevar != MYREP_FIELD_NULL)
        {
            switch ((*itr).typevar) {
            case MYREP_STRING:{
                QLineEdit *edit = static_cast<QLineEdit*>((*itr).widget );
                value = edit->text();
                break;
            }
            case MYREP_NUM:{
                QSpinBox *spin = static_cast<QSpinBox*>((*itr).widget );
                value = spin->text();
                break;
            }
            case MYREP_DATE:{
                QDateEdit *edit = static_cast<QDateEdit*>((*itr).widget );
                value = edit->text();
                break;
            }
            case MYREP_ENUM:{
                QComboBox *box = static_cast<QComboBox*>((*itr).widget );
                value = box->currentText();
                break;
            }
            }
            (*itr).value = value;
            (*itr).widget = 0;
            globalrepfiellist.append(*itr);
        }
        else
            value = (*itr).value;
        if(resobjdata.valueisquery) value = "'"+value+"'";
        resobjdata.value.replace((*itr).pos, (*itr).leng, value);
    }
    while(itr != resultlist.begin());

    return resobjdata;
}

bool MyDocumentODF::readAllZippedFile(QByteArray &data)
{
    QBuffer    buf;
    buf.setBuffer(&data);                                                       ///  подготовить источник данных

    int i,j;

    QuaZip zip(&buf);

    if(!zip.open(QuaZip::mdUnzip ) )
    {
        qDebug() << zip.getZipError();
        error = MYDOC_ZIPFILEERROR;
        return false;
    }

    for(bool f = zip.goToFirstFile(); f; f = zip.goToNextFile())
    {
        MyFileFromZip newfile;

        QuaZipFile zipfile(&zip);
        newfile.name = zip.getCurrentFileName();
        if(!zipfile.open(QIODevice::ReadOnly) )
        {
            zip.close();
            qDebug() << zipfile.errorString();
            error = MYDOC_ZIPFILEERROR;
            return false;
        }
        newfile.data = zipfile.readAll();
        files.append(newfile);

        if(newfile.name == "content.xml") i = files.size()-1;
        if(newfile.name == "styles.xml" ) j = files.size()-1;

        zipfile.close();
    }
    zip.close();

    contentfile = &files[i];
    stylesfile  = &files[j];

    if(contentfile == 0 || stylesfile == 0)
    {
        error = MYDOC_ZIPFILEERROR;
        return false;
    }
    return true;
}

bool MyDocumentODF::parsDocContent()
{
    if(!contentstruct.dom.setContent(contentfile->data) )
    {
        error = MYDOC_XMLSTRUCTERROR;
        return false;
    }

    if(!stylesstruct.dom.setContent(stylesfile->data) )
    {
        error = MYDOC_XMLSTRUCTERROR;
        return false;
    }

    /// очистка data
    contentfile->data.clear();
    stylesfile ->data.clear();

    /// парсинг документа content
    contentstruct.documentroot      = contentstruct.dom.firstChildElement("office:document-content");
    contentstruct.automatic_styles  = contentstruct.documentroot.firstChildElement("office:automatic-styles");
    contentstruct.body              = contentstruct.documentroot.firstChildElement("office:body");
    contentstruct.bodyroot          = contentstruct.body.firstChildElement("office:text");

    /// парсинг документа styles
    stylesstruct.documentroot    = stylesstruct.dom.firstChildElement("office:document-styles");
    stylesstruct.master_styles   = stylesstruct.documentroot.firstChildElement("office:master-styles");
    stylesstruct.master_page     = stylesstruct.master_styles.firstChildElement("style:master-page");
    stylesstruct.header          = stylesstruct.master_page.firstChildElement("style:header");
    stylesstruct.footer          = stylesstruct.master_page.firstChildElement("style:footer");

    return true;
}

/** создание таблицы **/

void MyDocumentODF::createTable(MyFileStructure& forfile,
                                QSqlQuery& querydata,
                                const QList<int>& fields,
                                const QStringList& fieldsname)
{
    int colcount  = fields.size();
    //int rowcount = querydata.size();

    double tablewidth = 17;                     // по умолчанию
    double colwidth   = tablewidth / colcount;

    QString tablestylename   = "MyTableStyle";
    QString tablecolumnstyle = "MyTableColumnStyle";
    QString tablecellstyle   = "MyTableCellStyle";

    /// описание стилей
    /// таблица
    QDomElement tablestyle = forfile.dom.createElement("style:style");
    tablestyle.setAttribute("style:name", tablestylename);
    tablestyle.setAttribute("style:family","table");
    QDomElement tableproperties = forfile.dom.createElement("style:table-properties");
    tableproperties.setAttribute("style:width", tablewidth);
    //tableproperties.setAttribute("table:align", align);
    tablestyle.appendChild(tableproperties);
    forfile.automatic_styles.appendChild(tablestyle);
    /// колонки
    QDomElement columnstyle = forfile.dom.createElement("style:style");
    columnstyle.setAttribute("style:name", tablecolumnstyle);
    columnstyle.setAttribute("style:family","table-column");
    QDomElement columnproperties = forfile.dom.createElement("style:table-column-properties");
    columnproperties.setAttribute("style:column-width", QString::number(colwidth)+"cm");
    columnproperties.setAttribute("style:rel-column-width", "6553*");
    columnstyle.appendChild(columnproperties);
    forfile.automatic_styles.appendChild(columnstyle);
    /// ячейки
    QDomElement cellstyle = forfile.dom.createElement("style:style");
    cellstyle.setAttribute("style:name",    tablecellstyle);
    cellstyle.setAttribute("style:family",  "table-cell");
    QDomElement cellproperties = forfile.dom.createElement("style:table-cell-properties");
    cellproperties.setAttribute("fo:padding",      "0.097cm");
    cellproperties.setAttribute("fo:border-right", "0.05pt solid #000000");
    cellproperties.setAttribute("fo:border-left",  "0.05pt solid #000000");
    cellproperties.setAttribute("fo:border-top",   "0.05pt solid #000000");
    cellproperties.setAttribute("fo:border-bottom","0.05pt solid #000000");
    cellstyle.appendChild(cellproperties);
    forfile.automatic_styles.appendChild(cellstyle);

    /// создание таблицы
    QDomElement mytable = forfile.dom.createElement("table:table");
    mytable.setAttribute("table:name","MyTable");
    mytable.setAttribute("table:style-name",tablestylename);
    /// описание столбцов
    for(int i = 0; i < colcount; ++i)
    {
        QDomElement mytablecolumn = forfile.dom.createElement("table:table-column");
        mytablecolumn.setAttribute("table:style-name",tablecolumnstyle);
        mytable.appendChild(mytablecolumn);
    }
    /// описание строк и ячеек строки
    querydata.first();
    /// создание шапки таблицы
    QDomElement mytablerow = forfile.dom.createElement("table:table-row");

    int colindex = 0;
    for(int c = 0; c < colcount; ++c)
    {
        colindex = fields[c];
        // ячейка шапки
        QDomElement mytablecell = forfile.dom.createElement("table:table-cell");
        mytablecell.setAttribute("table:style-name",tablecellstyle);
        mytablecell.setAttribute("office:value-type","string");
        // контент ячейки шапки
        QDomElement celltext = forfile.dom.createElement("text:p");
        // реальное значение ячейки
        QString value = fieldsname[c];
        QDomText text = forfile.dom.createTextNode(value);
        celltext.appendChild(text);
        mytablecell.appendChild(celltext);

        mytablerow.appendChild(mytablecell);
    }
    mytable.appendChild(mytablerow);
    /// заполнение таблицы значениями
    do// пройти все записи
    {
        QDomElement mytablerow = forfile.dom.createElement("table:table-row");

        int colindex = 0;
        for(int c = 0; c < colcount; ++c)
        {
            colindex = fields[c];
            // ячейка
            QDomElement mytablecell = forfile.dom.createElement("table:table-cell");
            mytablecell.setAttribute("table:style-name",tablecellstyle);
            mytablecell.setAttribute("office:value-type","string");
            // контент ячейки
            QDomElement celltext = forfile.dom.createElement("text:p");
            // реальное значение ячейки
            QString value = querydata.value(colindex).toString();
            QDomText text = forfile.dom.createTextNode(value);
            celltext.appendChild(text);
            mytablecell.appendChild(celltext);

            mytablerow.appendChild(mytablecell);
        }
        mytable.appendChild(mytablerow);
    }
    while(querydata.next() );

    forfile.bodyroot.appendChild(mytable);
}
