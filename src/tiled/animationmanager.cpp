/*
 * animationmanager.cpp
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#include "animationmanager.h"


#include "mapdocument.h"
#include "documentmanager.h"
#include "tileset.h"

#include <limits>

using namespace Tiled;
using namespace Tiled::Internal;

AnimationManager *AnimationManager::mInstance = 0;

AnimationManager *AnimationManager::instance()
{
    if (!mInstance)
        mInstance = new AnimationManager;
    return mInstance;
}

void AnimationManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

AnimationManager::AnimationManager()
{
    connect(DocumentManager::instance(), SIGNAL(currentDocumentChanged(MapDocument*)),
            this, SLOT(documentChanged(MapDocument*)));

    mAnimationTimer.setInterval(10);
    mAnimationTimer.setSingleShot(false);
    connect(&mAnimationTimer, SIGNAL(timeout()),
            this, SLOT(animationUpdate()));
    mAnimationTimer.start();
}

AnimationManager::~AnimationManager()
{

}

void AnimationManager::addTileset(Tileset *tileset)
{
    mTilesets.insert(tileset);
}

void AnimationManager::removeTileset(Tileset *tileset)
{
    mTilesets.remove(tileset);
}

void AnimationManager::animationUpdate()
{
    foreach (Tileset *ts, mTilesets) {
        ts->updateTime(mAnimationTimer.interval() / 10);
    }
    int waitNow = std::numeric_limits<int>::max();
    foreach (Tileset *ts, mTilesets) {
        const int duration = ts->duration();
        if (duration < waitNow)
            waitNow = duration;
    }
    mAnimationTimer.setInterval(waitNow * 10);
    emit animationChanged();
}

void AnimationManager::documentChanged(MapDocument *mapDocument)
{
    this->disconnect();
    if (mapDocument)
        connect(this, SIGNAL(animationChanged()),
                mapDocument, SLOT(emitRepaintAnimatedRegion()));
}
