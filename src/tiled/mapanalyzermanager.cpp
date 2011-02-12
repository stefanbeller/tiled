/*
 * mapanalyzermanager.cpp
 * Copyright 2011, Stefan Beller, stefanbeller@googlemail.com
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

#include "mapanalyzermanager.h"
#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

MapAnalyzerManager *MapAnalyzerManager::mInstance = 0;

MapAnalyzerManager::MapAnalyzerManager(QObject *parent)
    : QObject(parent)
{
}

MapAnalyzerManager::~MapAnalyzerManager()
{
}


void MapAnalyzerManager::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
}

MapAnalyzerManager *MapAnalyzerManager::instance()
{
    if (!mInstance)
        mInstance = new MapAnalyzerManager(0);

    return mInstance;
}

void MapAnalyzerManager::analyze()
{
    Map *map = mMapDocument->map();
    foreach (Layer* layer, map->layers()) {
        if (!layer->asTileLayer())
            continue;
        TileLayer *tl = static_cast<TileLayer*> (layer);
        for (int x = tl->bounds().left(); x < tl->bounds().right(); x++)
            for (int y = tl->bounds().top(); x < tl->bounds().bottom(); y++) {
                // you shall not pass
            }
    }
}
