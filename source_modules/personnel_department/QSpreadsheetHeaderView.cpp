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
    hideCol = menu->addAction(QIcon(":/svg/eye-icon.svg"),"Скрыть столбец");
    sortAZ = menu->addAction(QIcon(":/svg/sortdown-icon.svg"),
                             "Сортировка по убыванию");
    sortZA = menu->addAction(QIcon(":/svg/sortup-icon.svg"),
                             "Сортировка по возрастанию");
    filter = menu->addAction(QIcon(":/svg/filter-icon.svg"),"Быстрый фильтр");

    /// прорисовка SVG изображения
    QSvgRenderer renderer(QString(":/svg/menu-icon.svg"));
    svgImage = new QImage(40, 40, QImage::Format_ARGB32);
    svgImage->fill(QColor(246,246,246));
    QPainter imgpainter(svgImage);
    renderer.render(&imgpainter);

    prevlogicalIndex = -1;
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
        if (res == sortAZ) {}            
            emit sortDown(colnum, colname);
        if (res == sortZA) {}
            emit sortUp(colnum, colname);

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
