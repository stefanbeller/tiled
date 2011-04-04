/*
 * smarttilingmanager.h
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

#ifndef SMARTTILINGMANAGER_H
#define SMARTTILINGMANAGER_H

#include <QObject>
#include <QHash>

#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"

namespace Tiled {
namespace Internal {

class SmartTilingManager : public QObject
{
    Q_OBJECT
public:
    static SmartTilingManager *instance();
    static void deleteInstance();

    void setMapDocument(MapDocument *mapDocument);

    /*deprecated, but same as addSmartTiles, bound to a whole tileset*/
    //void addTileset(Tileset *ts);
    /*deprecated*/
    //void addTile(Cell t);

    /**
     * Adds all tiles found within the stamp tile layer to the smarttiling
     * manager. Between each of these tiles the relation in each direction is
     * calculated as well
     */
    void addSmartTiles(TileLayer *stamp);
    /**
     * Returns the relation between two tiles in horizontal mode. If these two
     * tiles are not found within mHProb the return value is assumed to be as
     * bad as possible.
     * This is INT_MAX/16. (a huge number, but since some of these numbers are
     * summed up, leave some even huger numbers, so we don't get overflows)
     */
    unsigned int getTileRelationH(Cell left, Cell right);

    /**
     * Returns the relation between two tiles in vertical mode. If these two
     * tiles are not found within mHProb the return value is assumed to be as
     * bad as possible.
     * This is INT_MAX/16. (a huge number, but since some of these numbers are
     * summed up, leave some even huger numbers, so we don't get overflows)
     */
    unsigned int getTileRelationV(Cell top, Cell bottom);

    /**
     * Returns the inverse probablitiy of a tile. This is found in mTileProb.
     * The probablitiy is inverse proportional of the needed weight to replace
     * that given tile
     */
    unsigned int getTileProb(Cell t);

    void setTileRelationH(Cell left, Cell right, quint64 value);
    void setTileRelationV(Cell top, Cell bottom, quint64 value);
    void setTileProb(Cell t, quint64 value);

public slots:
    /**
     * When triggered, smarttilin will start around the QRegion reg and will go
     * as far as it is allowed to change the terrain.
     */
    void smarttiling(QRegion reg, Layer *l);

    /**
     * This completely disables the smarttiling manager.
     */
    void setEnabled(bool en) { mEnabled = en; }

private:
    Q_DISABLE_COPY(SmartTilingManager)

    /**
     * Constructor. Only used by the SmartTiling manager itself.
     */
    explicit SmartTilingManager(QObject *parent = 0);

    /**
     * Destructor.
     */
    ~SmartTilingManager();

    /**
     * static pointer to the one and only instance of the smarttiling manager.
     */
    static SmartTilingManager *mInstance;


    /**
     * This will examine two given tiles and it will fill the given lists with
     * the border pixels of the two tiles.
     */
    void pixelLists(Cell first, Cell second, bool vertical,
                    int distFirst, int distSecond,
                    QList<QRgb> *destFirst, QList<QRgb> *destSecond);

    /**
     * calculates the number of lines taken into account for calculating the
     * relation between two tiles.
     */
    int numberOfRelationLines(Cell first, Cell second, bool vertical);

    /**
     * Calculates the 'similarity' of the given tiles, i. e. how well these two
     * tiles fit together when placed as neighbouring tiles.
     *
     * In orthogonal maps:
     * If vertical is true, the first tile is considered to be top of the
     * second. If vertical is false, the first tile is considered to be left of
     * the second.
     */
    void calculateRelation(Cell first, Cell second, bool vertical);

    /**
     * tries to find some better fitting tile at the given position within the
     * tile layer given. The ignored region marks the region, where smarttiling
     * already has done changes and should be ignored therefore.
     * If something is changed, that is added to the working region.
     * Returns true, if something was changed.
     */
    bool calculateTile(int x, int y, TileLayer *l,
                       QRegion *workingRegion,
                       QRegion ignoredRegion,
                       QRegion weightRegion);

    QHash<QPair<Cell,Cell>, quint64> mVProb;
    QHash<QPair<Cell,Cell>, quint64> mHProb;
    QHash<Cell, quint64> mTileProb;
    MapDocument *mMapDocument;
    bool mEnabled;
};

} // namespace Internal
} // namespace Tiled

#endif // SmartTilingMANAGER_H
