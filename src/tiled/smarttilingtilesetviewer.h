/*
 * smarttilingtilesetviewer.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#ifndef SMARTTILINGTILESETVIEWER_H
#define SMARTTILINGTILESETVIEWER_H

#include "tile.h"
#include "tilelayer.h"
#include "tilesetmodel.h"

#include <QWidget>
#include <QTableView>

class QLabel;

namespace Tiled {
namespace Internal {

class MapDocument;
class Zoomable;

class SmartTilingTilesetViewer : public QTableView
{
    Q_OBJECT
public:
    enum SmartTilingTilesetViewerPosition {
        top,
        left,
        right,
        bottom
    };

    explicit SmartTilingTilesetViewer(SmartTilingTilesetViewerPosition pos,
                                      Tile *t,
                                      QWidget *parent = 0);

    Zoomable *zoomable() const { return mZoomable; }

    SmartTilingTilesetViewerPosition getPosition() const { return mPosition; }
    Cell getReferenceTile() const { return mReferenceTile; }
    void setLabel(QLabel *l) { mText = l; }

    /**
     * Convenience method that returns the model as a TilesetModel.
     */
    TilesetModel *tilesetModel() const
    { return static_cast<TilesetModel *>(model()); }

signals:

private slots:
    void MouseEvent(const QModelIndex &index);
    void adjustScale();
private:
    Zoomable *mZoomable;
    Cell mReferenceTile;
    SmartTilingTilesetViewerPosition mPosition;
    QLabel *mText;
};

} // namespace Internal
} // namespace Tiled

#endif // SmartTilingTILESETVIEWER_H
