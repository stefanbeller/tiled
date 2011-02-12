#ifndef MAPANALYZER_H
#define MAPANALYZER_H

#include "tile.h"
#include "tilelayer.h"
#include "map.h"

#include <QMap>
#include <QObject>

namespace Tiled {

class Cell;
class Layer;
class Map;
class TileLayer;
class Tileset;

namespace Internal {

class CellInfo
{
public:
    CellInfo(Cell cell)
        : mCell(cell)
        , mAbsoluteCount(0)
    {
    }

    ~CellInfo()
    {

    }

    void addOccurence(Map *map, int x, int y)
    {
        mAbsoluteCount++;
        Map *piece = new Map(map->orientation(), 3, 3,
                             map->tileWidth(),map->tileHeight());
        foreach (Layer *l, map->layers()) {
            if (TileLayer* tl = l->asTileLayer())
                piece->insertLayer(piece->layerCount(), tl->copy(x - 1, y - 1, 3, 3));
        }
        bool added = false;
        for (int i = 0; i < mOccurences.length(); i++) {
            bool equal = true;
            for (int j = 0; j < mOccurences[i].first->layerCount(); j++) {
                TileLayer *there = mOccurences[i].first->layerAt(j)->asTileLayer();
                TileLayer *here = piece->layerAt(j)->asTileLayer();
                if (there && here && !there->computeDiffRegion(here).isEmpty()) {
                    equal = false;
                    break;
                }
            }
            if (equal) {
                 mOccurences[i].second++;
                 while (i > 0 &&
                        mOccurences[i].second > mOccurences[i - 1].second) {
                     mOccurences.swap(i, i - 1);
                     i--;
                 }

                 added = true;
            }
        }
        if (!added) {
            mOccurences.append(QPair<Map*,int>(piece, 1));
        } else {
            delete piece;
        }
    }

    int count() const
    { return mAbsoluteCount; }

    QList<QPair<Map*,int> > occurences() const
    { return mOccurences;}

    Cell cell() const
    { return mCell; }

private:
    Cell mCell;
    int mAbsoluteCount;

    QList<QPair<Map*,int > > mOccurences;
};

class MapAnalyzer : public QObject
{
    Q_OBJECT

public:
    MapAnalyzer();
    ~MapAnalyzer();
    void analyze(Map *map);

    Map *getInfoMap();

    QList<CellInfo> getCellInfo() const
    { return mCellInfo; }

private:
    QList<CellInfo> mCellInfo;
    Map *mAnalyzedMap;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPANALYZER_H
