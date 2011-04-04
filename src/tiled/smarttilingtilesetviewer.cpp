/*
 * smarttilingtilesetviewer.cpp
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "smarttilingtilesetviewer.h"
#include "smarttilingmanager.h"
#include "tilesetmodel.h"
#include "tileset.h"
#include "zoomable.h"

#include <QAbstractItemDelegate>
#include <QHeaderView>
#include <QPainter>
#include <QLabel>

#include <QDebug>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

/**
 * The delegate for drawing tile items in the tileset view.
 */
class TileDelegate : public QAbstractItemDelegate
{
public:
    TileDelegate(SmartTilingTilesetViewer *SmartTilingTilesetViewer, QObject *parent = 0)
        : QAbstractItemDelegate(parent)
        , mSmartTilingTilesetViewer(SmartTilingTilesetViewer)
    { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

private:
    SmartTilingTilesetViewer *mSmartTilingTilesetViewer;
};

void TileDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    // Draw the tile image
    const QVariant display = index.model()->data(index, Qt::DisplayRole);
    const QPixmap tileImage = display.value<QPixmap>();

    if (mSmartTilingTilesetViewer->zoomable()->smoothTransform())
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

    painter->drawPixmap(option.rect.adjusted(0, 0, -1, -1), tileImage);


    // Draw the priority of the tile when in priority mode
    QString priorityStr;

    const TilesetModel *model = static_cast<const TilesetModel*>(index.model());
    Cell t(model->tileAt(index));
    SmartTilingManager *inst = SmartTilingManager::instance();
    Cell reference = mSmartTilingTilesetViewer->getReferenceTile();
    switch (mSmartTilingTilesetViewer->getPosition()) {
    case SmartTilingTilesetViewer::left:
        priorityStr.setNum(inst->getTileRelationH(t, reference));
        break;
    case SmartTilingTilesetViewer::right:
        priorityStr.setNum(inst->getTileRelationH(reference, t));
        break;
    case SmartTilingTilesetViewer::top:
        priorityStr.setNum(inst->getTileRelationV(t, reference));
        break;
    case SmartTilingTilesetViewer::bottom:
        priorityStr.setNum(inst->getTileRelationV(reference, t));
        break;
    }

    QRectF textPos = QRectF(option.rect);

    int textOffsetX = 12;
    int textOffsetY = 5;

    painter->setFont(QFont(QLatin1String("Tahoma"), 12));
    painter->setPen(QColor::fromRgb(0,0,0));
    painter->drawText(textPos.adjusted(textOffsetX, textOffsetY, -1, -1), priorityStr);
    painter->setPen(QColor::fromRgb(255,255,255));
    painter->drawText(textPos.adjusted(textOffsetX + 1, textOffsetY + 1, -1, -1), priorityStr);

    // Overlay with highlight color when selected
    if (option.state & QStyle::State_Selected) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.5);
        painter->fillRect(option.rect.adjusted(0, 0, -1, -1),
                          option.palette.highlight());
        painter->setOpacity(opacity);
    }
}

QSize TileDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                             const QModelIndex &index) const
{
    const TilesetModel *m = static_cast<const TilesetModel*>(index.model());
    const Tileset *tileset = m->tileset();
    const qreal zoom = mSmartTilingTilesetViewer->zoomable()->scale();

    return QSize(tileset->tileWidth() * zoom + 1,
                 tileset->tileHeight() * zoom + 1);
}

} // anonymous namespace

SmartTilingTilesetViewer::SmartTilingTilesetViewer(
    SmartTilingTilesetViewerPosition pos,
    Tile *t,
    QWidget *parent)
    : QTableView(parent)
    , mZoomable(new Zoomable(this))
    , mReferenceTile(t)
    , mPosition(pos)
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new TileDelegate(this, this));
    setShowGrid(false);

    QHeaderView *header = horizontalHeader();
    header->hide();
    header->setResizeMode(QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(1);

    header = verticalHeader();
    header->hide();
    header->setResizeMode(QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(1);

    // Hardcode this view on 'left to right' since it doesn't work properly
    // for 'right to left' languages.
    setLayoutDirection(Qt::LeftToRight);

    connect(mZoomable, SIGNAL(scaleChanged(qreal)), SLOT(adjustScale()));
    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(MouseEvent(QModelIndex)));

}

void SmartTilingTilesetViewer::MouseEvent(const QModelIndex &index)
{
    const TilesetModel *m = static_cast<const TilesetModel*>(index.model());
    Cell t(m->tileAt(index));

    int n;
    switch (mPosition) {
    case SmartTilingTilesetViewer::left:
        n = SmartTilingManager::instance()->getTileRelationH(t, mReferenceTile);
        break;
    case SmartTilingTilesetViewer::right:
        n = SmartTilingManager::instance()->getTileRelationH(mReferenceTile, t);
        break;
    case SmartTilingTilesetViewer::top:
        n = SmartTilingManager::instance()->getTileRelationV(t, mReferenceTile);
        break;
    case SmartTilingTilesetViewer::bottom:
        n = SmartTilingManager::instance()->getTileRelationV(mReferenceTile, t);
        break;
    }
    QString s;
    s.setNum(n);
    mText->setText(s);
}

void SmartTilingTilesetViewer::adjustScale()
{
    tilesetModel()->tilesetChanged();
}
