#include "mapanalyzer.h"

#include "objectgroup.h"

using namespace Tiled;
using namespace Tiled::Internal;

MapAnalyzer::MapAnalyzer()
    : QObject(0)
{
}

MapAnalyzer::~MapAnalyzer()
{
    for (int i = 0; i < mCellInfo.length(); i++)
        for (int j = 0; j < mCellInfo.at(i).occurences().length(); j++)
            delete mCellInfo.at(i).occurences().at(j).first;

}

void MapAnalyzer::analyze(Map *map)
{
    mAnalyzedMap = map;
    foreach (Layer* layer, map->layers()) {
        if (!layer->asTileLayer())
            continue;
        TileLayer *tl = static_cast<TileLayer*> (layer);
        const int xstart = tl->bounds().left() + 1;
        const int xend = tl->bounds().right() - 1;
        const int ystart = tl->bounds().top() + 1;
        const int yend = tl->bounds().bottom() - 1;
        for (int x = xstart; x < xend; x++) {
            for (int y = ystart; y < yend; y++) {
                const Cell checkedCell = tl->cellAt(x, y);
                if (!checkedCell.isEmpty()) {
                    bool added = false;
                    for (int i = 0; i < mCellInfo.length(); i++) {
                        if (mCellInfo.at(i).cell() == checkedCell) {
                            mCellInfo[i].addOccurence(map, x, y);
                            while (i > 0 && mCellInfo.at(i).count() > mCellInfo.at(i-1).count()) {
                                mCellInfo.swap(i, i - 1);
                                i--;
                            }
                            added = true;
                            break;
                        }
                    }
                    if (!added) {
                        CellInfo cellInfo(checkedCell);
                        cellInfo.addOccurence(map, x, y);
                        mCellInfo.append(cellInfo);
                    }
                }
            }
        }
    }
}
#include <QDebug>
Map *MapAnalyzer::getInfoMap()
{
    int maxOccurences = 0;
    for (int i = 0; i < mCellInfo.length(); i++)
        maxOccurences = qMax(mCellInfo.at(i).occurences().length(), maxOccurences);

    int height =  4 * mCellInfo.length() - 1;
    int width = 3 + 4*maxOccurences;
    Map *result = new Map(mAnalyzedMap->orientation(), width, height,
                          mAnalyzedMap->tileWidth(), mAnalyzedMap->tileHeight());
    TileLayer *workingLayer = new TileLayer(tr("exposed Layer"), 0, 0, width, height);
    foreach (Layer *l, mAnalyzedMap->layers()) {
        if (l->asTileLayer())
            result->addLayer(new TileLayer(l->name(), 0, 0, width, height));
        else
            result->addLayer(new ObjectGroup(l->name(), 0, 0, width, height));
    }

    for (int i = 0; i < mCellInfo.length(); i++) {
        const int x = 1;
        const int y = 1 + 4*i;
        Cell toSet =  mCellInfo.at(i).cell();
        toSet.setProperty(tr("absolute count"),
                          QString::number(mCellInfo.at(i).count()));
        workingLayer->setCell(x, y, toSet);

        for (int j = 0; j < mCellInfo[i].occurences().length(); j++) {
            for (int k = 0; k < mCellInfo[i].occurences().at(j).first->layerCount(); k++) {
                Layer *l = mCellInfo[i].occurences().at(j).first->layerAt(k);
                const int count = mCellInfo[i].occurences().at(j).second;
                if (TileLayer *tl = l->asTileLayer()) {
                    TileLayer *tldest = result->layerAt(k)->asTileLayer();
                    const int x = 4 * j + 3;
                    const int y = 4 * i;
                    tldest->merge(QPoint(x, y), tl);
                    workingLayer->getCellAt(x + 1, y + 1)->setProperty(tr("absolute count"),
                                                                       QString::number(count));
                }
            }
        }
    }

    result->addLayer(workingLayer);
    return result;
}
