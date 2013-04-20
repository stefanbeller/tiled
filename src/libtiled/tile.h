/*
 * tile.h
 * Copyright 2008-2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TILE_H
#define TILE_H

#include "object.h"

#include <QPixmap>

namespace Tiled {

class Terrain;
class Tileset;

/**
 * Returns the given \a terrain with the \a corner modified to \a terrainId.
 */
inline unsigned setTerrainCorner(unsigned terrain, int corner, int terrainId)
{
    unsigned mask = 0xFF << (3 - corner) * 8;
    unsigned insert = terrainId << (3 - corner) * 8;
    return (terrain & ~mask) | (insert & mask);
}

class TILEDSHARED_EXPORT Tile : public Object
{
public:
    Tile(const QPixmap &image, int id, Tileset *tileset):
        Object(TileType),
        mId(id),
        mTileset(tileset),
        mImage(image),
        mTerrain(-1),
        mTerrainProbability(-1.f),
        mIsAnimated(false),
        mDuration(0),
        mCurrentFrameIndex(0)
    {}

    /**
     * Returns ID of this tile within its tileset.
     */
    int id() const { return mId; }

    /**
     * Returns the tileset that this tile is part of.
     */
    Tileset *tileset() const { return mTileset; }

    /**
     * Returns the image of this tile.
     */
    const QPixmap &image() const;

    /**
     * Sets the image of this tile.
     */
    void setImage(const QPixmap &image) { mImage = image; }

    /**
     * Returns the width of this tile.
     */
    int width() const { return mImage.width(); }

    /**
     * Returns the height of this tile.
     */
    int height() const { return mImage.height(); }

    /**
     * Returns the size of this tile.
     */
    QSize size() const { return mImage.size(); }

    /**
     * Returns the Terrain of a given corner.
     */
    Terrain *terrainAtCorner(int corner) const;

    /**
     * Returns the terrain id at a given corner.
     */
    int cornerTerrainId(int corner) const { unsigned t = (terrain() >> (3 - corner)*8) & 0xFF; return t == 0xFF ? -1 : (int)t; }

    /**
     * Set the terrain type of a given corner.
     */
    void setCornerTerrain(int corner, int terrainId)
    { setTerrain(setTerrainCorner(mTerrain, corner, terrainId)); }

    /**
     * Returns the terrain for each corner of this tile.
     */
    unsigned terrain() const { return mTerrain; }

    /**
     * Set the terrain for each corner of the tile.
     */
    void setTerrain(unsigned terrain);

    /**
     * Returns the probability of this terrain type appearing while painting (0-100%).
     */
    float terrainProbability() const { return mTerrainProbability; }

    /**
     * Set the probability of this terrain type appearing while painting (0-100%).
     */
    void setTerrainProbability(float probability) { mTerrainProbability = probability; }

    bool isAnimated() const { return mIsAnimated; }

    /**
     * Only for animated tiles:
     * The duration in milliseconds until the next frame shows up,
     * if this tile is animated
     */
    int duration();

    /**
     * decreases the time counter until the next frame is shown
     * returns if the image has changed.
     */
    bool updateTime(int timePassed);

    bool checkForAnimationSequence();

private:
    int mId;
    Tileset *mTileset;
    QPixmap mImage;
    unsigned mTerrain;
    float mTerrainProbability;

    bool mIsAnimated;
    int mDuration;
    int mCurrentFrameIndex;
    QList<QPixmap*> mFrames;
    QList<int> mDurations;

};

} // namespace Tiled

#endif // TILE_H
