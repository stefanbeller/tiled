/*
 * smarttilingviewer.cpp
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

#include "smarttilingmanager.h"
#include "smarttilingviewer.h"
#include "smarttilingtilesetviewer.h"
#include "smarttilingtileviewer.h"
#include "tile.h"
#include "tileset.h"
#include "tilesetmodel.h"
#include "zoomable.h"

#include <QGridLayout>
#include <QPushButton>

using namespace Tiled;
using namespace Tiled::Internal;




SmartTilingViewer::SmartTilingViewer(const QString &kind,
                                   Tile *tile,
                                   QWidget *parent)
    : QDialog(parent)
    , mTile(tile)
    , mKind(kind)
{
    QWidget *grid = new QWidget(this);
    SmartTilingTilesetViewer *childleft = new SmartTilingTilesetViewer(SmartTilingTilesetViewer::left, tile, grid);
    SmartTilingTilesetViewer *childtop = new SmartTilingTilesetViewer(SmartTilingTilesetViewer::top, tile, grid);
    SmartTilingTilesetViewer *childbottom = new SmartTilingTilesetViewer(SmartTilingTilesetViewer::bottom, tile, grid);
    SmartTilingTilesetViewer *childright = new SmartTilingTilesetViewer(SmartTilingTilesetViewer::right, tile, grid);
    SmartTilingTileViewer *center = new SmartTilingTileViewer(tile, grid);

    Tileset *tileset = tile->tileset();
    childleft->setModel(new TilesetModel(tileset, childleft));
    childright->setModel(new TilesetModel(tileset, childright));
    childtop->setModel(new TilesetModel(tileset, childtop));
    childbottom->setModel(new TilesetModel(tileset, childbottom));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(childleft, 1, 0, 0);
    layout->addWidget(childright, 1, 2, 0);
    layout->addWidget(childtop, 0, 1, 0);
    layout->addWidget(childbottom, 2, 1, 0);
    layout->addWidget(center, 1, 1, 0);

    QPushButton *calc = new QPushButton(tr("&Calculate Relations"), grid);
    layout->addWidget(calc,2,2,0);
    connect(calc, SIGNAL(clicked()), this, SLOT(recalculateClicked()));

    QLabel *l = new QLabel(grid);
    layout->addWidget(l, 3, 2, 0);
    childleft->setLabel(l);
    childright->setLabel(l);
    childtop->setLabel(l);
    childbottom->setLabel(l);
}

SmartTilingViewer::~SmartTilingViewer()
{

}

void SmartTilingViewer::recalculateClicked()
{
    //SmartTilingManager::instance()->addTileset(mTile->tileset());
}
