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

/*!
    исходный класс был переделан под мои нужды.
    изменен механизм перерисовки секций хедера
*/

#ifndef QSPREADSHEETHEADERVIEW_H
#define QSPREADSHEETHEADERVIEW_H

#include <QHeaderView>

class QSpreadsheetHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    QSpreadsheetHeaderView(Qt::Orientation orientation, QWidget * parent = 0);

signals:
    void sortUp     (int column, QString name);                                 /// сигнал сортировки по возрастанию
    void sortDown   (int column, QString name);                                 /// по убыванию

private:
    /// обработка событий
    void mousePressEvent    (QMouseEvent * event);
    void mouseMoveEvent     (QMouseEvent * event);
    void paintSection       (QPainter *painter, const QRect &rect,
                             int logicalIndex) const;

    /// прорисовка дополнительных элементов
    void drawMenuButton (QPainter *painter, int logicalIndex,
                        bool enabled) const;
    void drawPrevButton (QPainter *painter, int logicalIndex) const;
    void drawNextButton (QPainter *painter, int logicalIndex) const;

    QRect sectionRect   (int logicalIndex) const;                               /// получить плоскость заголовка столбца
    QRect buttonMenuRect(int logicalIndex) const;                               /// получить плоскость кнопки меню
    QRect prevRect      (int logicalIndex) const;                               /// получить плоскость кнопки "<"
    QRect nextRect      (int logicalIndex) const;                               /// получить плоскость кнопки ">"

    int prevlogicalIndex;                                                       /// индекс предыдущего выбранного хедера

    /// объекты меню
    QMenu*   menu;
    QAction* hideCol;
    QAction* sortAZ;
    QAction* sortZA;
    QAction* filter;
    QImage*  svgImage;
};

#endif /* QSPREADSHEETHEADERVIEW_H */
