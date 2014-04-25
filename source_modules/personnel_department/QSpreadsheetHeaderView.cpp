/*
*
* This file is part of QSpreadsheetHeaderView,
* an open-source header view widget
*
* Copyright (C) 2012 Morgan Leborgne
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with QSpreadsheetHeaderView. If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <QPainter>
#include <QCursor>
#include <QHoverEvent>
#include <QMenu>
#include <QtSvg>

#include <QDebug>

#include "QSpreadsheetHeaderView.h"

QSpreadsheetHeaderView::QSpreadsheetHeaderView(Qt::Orientation orientation,
                                               QWidget * parent)
    : QHeaderView(orientation, parent)
{
    setAttribute(Qt::WA_Hover, true);                                           /// установка атрибута, обеспечивающего перерисовку компонента при покидании и наведении мыши

    menu = new QMenu(this);                                                     /// создание меню

    icoAZ = new QIcon(":/svg/sortdown-icon.svg");
    icoZA = new QIcon(":/svg/sortup-icon.svg");

    AZmap = new QPixmap(":/png/down.png");
    ZAmap = new QPixmap(":/png/up.png");;

    hideCol = menu->addAction(QIcon(":/svg/eye-icon.svg"),    "Скрыть столбец");
    sortAZ  = menu->addAction(*icoAZ,  "Сортировка по убыванию");
    sortZA  = menu->addAction(*icoZA,  "Сортировка по возрастанию");
    resOrder= menu->addAction("Отменить сортировку всех полей");

    /// прорисовка SVG изображения
    QSvgRenderer renderer(QString(":/svg/menu-icon.svg") );
    svgImage = new QImage(20, 20, QImage::Format_ARGB32);
    svgImage->fill(QColor(246,246,246));
    QPainter imgpainter(svgImage);
    imgpainter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer.render(&imgpainter);
    setDefaultAlignment(Qt::AlignLeft);

    prevlogicalIndex = -1;
}

void QSpreadsheetHeaderView::setCustomOrderData(QMap<int, QString> data)
{
    customOrder = data;
}

QSize QSpreadsheetHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize ressize = QHeaderView::sectionSizeFromContents(logicalIndex);
    ressize.setWidth(ressize.width()+25);

    return ressize;
}

void QSpreadsheetHeaderView::setSortUp(int column, QString name)
{
    if(!customOrder.contains(column) ) customOrder.insert(column, "ASC");
    else customOrder[column] = "ASC";
    emit sortDown(column+1, name);
}

void QSpreadsheetHeaderView::setSortDown(int column, QString name)
{
    if(!customOrder.contains(column) ) customOrder.insert(column, "DESC");
    else customOrder[column] = "DESC";
    emit sortUp(column+1, name);
}

void QSpreadsheetHeaderView::clearOrders()
{
    customOrder.clear();
    emit orderClear();
}

void QSpreadsheetHeaderView::mousePressEvent ( QMouseEvent * event )
{
    QHeaderView::mousePressEvent(event);

    int logicalIndex = logicalIndexAt(event->pos());                            /// возвращает индекс объекта над которым сработало событие мыши

    /// костыль для правильной перерисовки хедера
    if(prevlogicalIndex != -1) updateSection(prevlogicalIndex);                 /// обязательно обновить предыдущую секцию
    prevlogicalIndex = logicalIndex;
    updateSection(logicalIndex);

    if (buttonMenuRect(logicalIndex).contains(event->pos()))                    /// если курсор над кнопкой меню, создается меню
    {
        hideCol->setEnabled(hiddenSectionCount() < count() - 1);                /// установить активность пункта "спрятать столбец"

        QAction *res = menu->exec(mapToGlobal(event->pos()));                   /// отображает меню в текущих глобальных координатах

        int colnum = visualIndex(logicalIndex);                                 /// номер столбца
        QString colname = model()->headerData(colnum,orientation()).toString(); /// наименование столбца


        if (res == hideCol) {
            hideSection(logicalIndex);
            updateSection(logicalIndex-1);
        }
        if (res == sortAZ) setSortUp  (colnum, colname);
        if (res == sortZA) setSortDown(colnum, colname);
        if (res == resOrder) clearOrders();

        updateSection(logicalIndex);                                            /// выполнил действие - обнови секцию!
    }

    if (prevRect(logicalIndex).contains(event->pos()))                          /// отобразить скрытый столбец по нажатию "<"
    {
        showSection(logicalIndex - 1);
        updateSection(logicalIndex - 2);
    }

    if (nextRect(logicalIndex).contains(event->pos()))                          /// отобразить скрытый столбец по нажатию ">"
    {
        showSection(logicalIndex + 1);
        updateSection(logicalIndex + 2);
    }

}

void QSpreadsheetHeaderView::mouseMoveEvent(QMouseEvent * event)
{
    QHeaderView::mouseMoveEvent(event);

    int logicalIndex = logicalIndexAt(event->pos());
    updateSection(logicalIndex);
}

void QSpreadsheetHeaderView::paintSection(QPainter *painter, const QRect &rect,
                                          int logicalIndex) const
{
    painter->save();

    QHeaderView::paintSection(painter, rect, logicalIndex);

    painter->restore();

    if (!rect.isValid() || isSortIndicatorShown())
        return;

    if(customOrder.contains(logicalIndex+1))
    {
        if(customOrder[logicalIndex+1] == "ASC" ||
           customOrder[logicalIndex+1] == "DESC" )
            drawOrderSign(painter, logicalIndex);
    }

    if (isSectionHidden(logicalIndex - 1)) {
        drawPrevButton(painter, logicalIndex);
    }

    if (isSectionHidden(logicalIndex + 1)) {
        drawNextButton(painter, logicalIndex);
    }

    QPoint pos = this->mapFromGlobal(QCursor::pos());
    if (rect.contains(pos)) {
        drawMenuButton(painter, logicalIndex,
                       buttonMenuRect(logicalIndex).contains(pos));
    }
}

QRect QSpreadsheetHeaderView::sectionRect(int logicalIndex) const
{
    return QRect(sectionViewportPosition(logicalIndex), 0,
                 sectionSize(logicalIndex), height());
}

QRect QSpreadsheetHeaderView::buttonMenuRect(int logicalIndex) const
{
    QRect sr = sectionRect(logicalIndex);

    return QRect(sr.right() -20, sr.center().y() - 8, 16, 16);
}

QRect QSpreadsheetHeaderView::prevRect(int logicalIndex) const
{
    if (isSectionHidden(logicalIndex))
        return QRect();

    QRect sr = sectionRect(logicalIndex);

    return QRect(sr.left() + 1, sr.center().y() - 6, 13, 13);
}

QRect QSpreadsheetHeaderView::nextRect(int logicalIndex) const
{
    if (isSectionHidden(logicalIndex))
        return QRect();

    QRect sr = sectionRect(logicalIndex);

    return QRect(sr.right() - 13, sr.center().y() - 6, 13, 13);
}

QRect QSpreadsheetHeaderView::signOrd(int logicalIndex) const
{
    QRect sr = buttonMenuRect(logicalIndex);
    return QRect(sr.left()+4, sr.top()+4, 7, 7);
}

void QSpreadsheetHeaderView::drawMenuButton(QPainter *painter, int logicalIndex,
                                            bool enabled) const
{
    QRect brect = buttonMenuRect(logicalIndex);

    painter->setPen(enabled ? QColor(186,186,186) : QColor(223, 223, 223));
    painter->setBrush(QColor(246,246,246));
    painter->drawRect(brect);

    painter->drawImage(brect.adjusted(1,1,0,0), *svgImage);
}

void QSpreadsheetHeaderView::drawPrevButton(QPainter *painter,
                                            int logicalIndex) const
{
    QRect rect = prevRect(logicalIndex);

    painter->setPen(QColor(71,71,71));
    painter->drawLine(rect.left()+1, rect.center().y() - 3, rect.left()+1,
                      rect.center().y() + 3);
    painter->drawLine(rect.left()+2, rect.center().y() - 2, rect.left()+2,
                      rect.center().y() + 2);
    painter->drawLine(rect.left()+3, rect.center().y() - 1, rect.left()+3,
                      rect.center().y() + 1);
    painter->drawPoint(rect.left()+4, rect.center().y());
}

void QSpreadsheetHeaderView::drawNextButton(QPainter *painter,
                                            int logicalIndex) const
{
    QRect rect = nextRect(logicalIndex);

    painter->setPen(QColor(71,71,71));
    painter->drawLine(rect.right()-2, rect.center().y() - 3, rect.right()-2,
                      rect.center().y() + 3);
    painter->drawLine(rect.right()-3, rect.center().y() - 2, rect.right()-3,
                      rect.center().y() + 2);
    painter->drawLine(rect.right()-4, rect.center().y() - 1, rect.right()-4,
                      rect.center().y() + 1);
    painter->drawPoint(rect.right()-5, rect.center().y());
}

void QSpreadsheetHeaderView::drawOrderSign(QPainter *painter, int logicalIndex) const
{
    QRect ordrect = signOrd(logicalIndex);
    painter->setPen(QPen(QBrush(),0) );

    painter->setRenderHint(QPainter::HighQualityAntialiasing,   true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,     true);

    if(customOrder[logicalIndex+1] == "ASC")  painter->drawPixmap(ordrect, *AZmap);
    if(customOrder[logicalIndex+1] == "DESC") painter->drawPixmap(ordrect, *ZAmap);
}
