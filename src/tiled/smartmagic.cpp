#include "smartmagic.h"

#include "mapdocument.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

SmartMagic::SmartMagic()
{
}

SmartMagic::~SmartMagic()
{
}

void SmartMagic::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
}

void SmartMagic::setMapAnalyzer(MapAnalyzer *mapAnalyzer)
{
    mMapAnalyzer = mapAnalyzer;
}

void SmartMagic::applyMagic(QRegion r, Layer* where)
{
    if (r.boundingRect().width() != 1 || r.boundingRect().height() != 1)
        return;

    TileLayer *editedLayer = where->asTileLayer();
    if (!editedLayer)
        return;

    const Map *map = mMapDocument->map();
    const int x = r.boundingRect().left();
    const int y = r.boundingRect().top();
    const QPoint offset(x - 1, y - 1);

    Cell whichCell = editedLayer->cellAt(x, y);

    QList<CellInfo> cellInfoList = mMapAnalyzer->getCellInfo();

    const CellInfo *cellInfo;

    for (int i = 0; i < cellInfoList.length(); i++)
        if (whichCell == cellInfoList.at(i).cell())
            cellInfo = &cellInfoList.at(i);

    if (!cellInfo)
        return;

    QList<QPair<Map*,int> > occ = cellInfo->occurences();
    int dhmin = INT_MAX;
    const Map* dhmin_map = 0;
    for (int i = 0; i < occ.length(); i++) {
        const Map* compareMap = occ[i].first;
        int dh = 0;
        for (int j = 0; j < compareMap->layerCount(); j++) {
            QRegion diffRegion = compareMap->layerAt(j)->asTileLayer()->computeDiffRegion(map->layerAt(j)->asTileLayer());
            foreach (QRect r, diffRegion.rects())
                dh += r.width() * r.height();
        }
        if (dh < dhmin) {
            dhmin = dh;
            dhmin_map = compareMap;
        }
    }
    for (int j = 0; j < map->layerCount(); j++) {
        map->layerAt(j)->asTileLayer()->merge(offset, dhmin_map->layerAt(j)->asTileLayer());
    }
}
