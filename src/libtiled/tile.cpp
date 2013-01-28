/*
 * tile.cpp
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#include "tile.h"

#include "tileset.h"
#include <limits>

using namespace Tiled;

Terrain *Tile::terrainAtCorner(int corner) const
{
    return mTileset->terrain(cornerTerrainId(corner));
}

void Tile::setTerrain(unsigned terrain)
{
    if (mTerrain == terrain)
        return;

    mTerrain = terrain;
    mTileset->markTerrainDistancesDirty();
}

const QPixmap &Tile::image() const
{
    if (!mIsAnimated) {
        return mImage;
    } else {
        return *mFrames[mCurrentFrameIndex];
    }
}

int Tile::duration()
{
    if (mIsAnimated)
        return mDuration;
    else
        return std::numeric_limits<int>::max();
}

bool Tile::updateTime(int timePassed)
{
    if (!mIsAnimated)
        return false;

    bool changedImage = false;

    mDuration -= timePassed;
    while (mDuration <= 0) {
        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrames.size();
        mDuration += mDurations[mCurrentFrameIndex];
        changedImage = true;
    }
    return changedImage;
}

bool Tile::checkForAnimationSequence()
{
    mIsAnimated = false;
    for (int i = 0; ;i++) {
        const QString frameName = QLatin1String("animation-frame") + QString::number(i);
        const QString delayName = QLatin1String("animation-delay") + QString::number(i);

        if (properties().contains(frameName) && properties().contains(delayName)) {
            int frame = property(frameName).toInt();
            int delay = property(delayName).toInt();
            mFrames.append(&mTileset->tileAt(frame)->mImage);
            mDurations.append(delay);
            mIsAnimated = true;
        } else {
            break;
        }
    }
    return mIsAnimated;
}


