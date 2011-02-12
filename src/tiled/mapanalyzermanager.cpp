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

#include "documentmanager.h"
#include "mapanalyzer.h"
#include "map.h"
#include "mapdocument.h"
#include "smartmagic.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

MapAnalyzerManager *MapAnalyzerManager::mInstance = 0;

MapAnalyzerManager::MapAnalyzerManager(QObject *parent)
    : QObject(parent)
    , mMapAnalyzer(new MapAnalyzer())
    , mSmartMagic(new SmartMagic())
{
    mSmartMagic->setMapAnalyzer(mMapAnalyzer);
}

MapAnalyzerManager::~MapAnalyzerManager()
{
    delete mMapAnalyzer;
}

void MapAnalyzerManager::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
    mSmartMagic->setMapDocument(mMapDocument);
    connect(mMapDocument, SIGNAL(regionEdited(QRegion,Layer*)),
            mSmartMagic, SLOT(applyMagic(QRegion,Layer*)));
}

MapAnalyzerManager *MapAnalyzerManager::instance()
{
    if (!mInstance)
        mInstance = new MapAnalyzerManager(0);

    return mInstance;
}

void MapAnalyzerManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

void MapAnalyzerManager::analyze()
{
    mMapAnalyzer->analyze(mMapDocument->map());
    Map *analysis = mMapAnalyzer->getInfoMap();

    DocumentManager *dm = DocumentManager::instance();
    dm->addDocument(new MapDocument(analysis, QString()));
}

QList<CellInfo> MapAnalyzerManager::getAnalysis()
{
    return mMapAnalyzer->getCellInfo();
}


