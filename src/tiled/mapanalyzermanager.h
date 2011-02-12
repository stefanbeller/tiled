/*
 * mapanalyzermanager.h
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

#ifndef MAPANALYZERMANAGER_H
#define MAPANALYZERMANAGER_H

#include <QObject>
#include "mapanalyzer.h"

namespace Tiled {

class Layer;
class Map;
class TileLayer;
class Tileset;

namespace Internal {

class MapDocument;
class MapAnalyzer;
class SmartMagic;

class MapAnalyzerManager: public QObject
{
    Q_OBJECT

public:
    /**
     * Requests the map analyzer manager. When the manager doesn't exist
     * yet, it will be created.
     */
    static MapAnalyzerManager *instance();

    /**
     * Deletes the Mapanalysing manager instance, when it exists.
     */
    static void deleteInstance();

    void analyze();

    void setMapDocument(MapDocument *mapDocument);

    QList<CellInfo> getAnalysis();

private:
    Q_DISABLE_COPY(MapAnalyzerManager)

    /**
     * Constructor. Only used by the AutomaticMapping manager itself.
     */
    MapAnalyzerManager(QObject *parent);

    ~MapAnalyzerManager();

    /**
     * The current map document.
     */
    MapDocument *mMapDocument;

    static MapAnalyzerManager *mInstance;

    MapAnalyzer *mMapAnalyzer;
    SmartMagic *mSmartMagic;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPANALYZERMANAGER_H
