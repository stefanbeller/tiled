#include "colourpainter.h"

#include "mapdocument.h"
#include "colourlayer.h"
#include "map.h"

using namespace Tiled;
using namespace Tiled::Internal;

ColourPainter::ColourPainter(MapDocument *mapDocument, ColourLayer *colourLayer)
    : mMapDocument(mapDocument)
    , mColourLayer(colourLayer)
{
}

QColor ColourPainter::cellAt(int x, int y) const
{
    const int layerX = x - mColourLayer->x();
    const int layerY = y - mColourLayer->y();

    if (!mColourLayer->contains(layerX, layerY))
        return QColor();

    return mColourLayer->cellAt(layerX, layerY);
}

void ColourPainter::setCell(int x, int y, const QColor &cell)
{
    const QRegion &selection = mMapDocument->tileSelection();
    if (!(selection.isEmpty() || selection.contains(QPoint(x, y))))
        return;

    const int layerX = x - mColourLayer->x();
    const int layerY = y - mColourLayer->y();

    if (!mColourLayer->contains(layerX, layerY))
        return;

    mColourLayer->setCell(layerX, layerY, cell);
    mMapDocument->emitRegionChanged(QRegion(x, y, 1, 1));
}

void ColourPainter::setCells(int x, int y,
                           ColourLayer *colourLayer,
                           const QRegion &mask)
{
    QRegion region = paintableRegion(x, y,
                                     colourLayer->width(),
                                     colourLayer->height());
    if (!mask.isEmpty())
        region &= mask;
    if (region.isEmpty())
        return;

    mColourLayer->setCells(x - mColourLayer->x(),
                         y - mColourLayer->y(),
                         colourLayer,
                         region.translated(-mColourLayer->position()));

    mMapDocument->emitRegionChanged(region);
}

void ColourPainter::drawCells(int x, int y, ColourLayer *colourLayer)
{
    const QRegion region = paintableRegion(x, y,
                                           colourLayer->width(),
                                           colourLayer->height());
    if (region.isEmpty())
        return;

    foreach (const QRect &rect, region.rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {
                const QColor &cell = colourLayer->cellAt(_x - x, _y - y);
                if (cell.alpha()==0)
                    continue;

                mColourLayer->setCell(_x - mColourLayer->x(),
                                    _y - mColourLayer->y(),
                                    cell);
            }
        }
    }

    mMapDocument->emitRegionChanged(region);
}

void ColourPainter::drawStamp(const ColourLayer *stamp,
                            const QRegion &drawRegion)
{
    Q_ASSERT(stamp);
    if (stamp->bounds().isEmpty())
        return;

    const QRegion region = paintableRegion(drawRegion);
    if (region.isEmpty())
        return;

    const int w = stamp->width();
    const int h = stamp->height();
    const QRect regionBounds = region.boundingRect();

    foreach (const QRect &rect, region.rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {
                const int stampX = (_x - regionBounds.left()) % w;
                const int stampY = (_y - regionBounds.top()) % h;
                const QColor &cell = stamp->cellAt(stampX, stampY);
                if (cell.alpha() == 0)
                    continue;

                mColourLayer->setCell(_x - mColourLayer->x(),
                                    _y - mColourLayer->y(),
                                    cell);
            }
        }
    }

    mMapDocument->emitRegionChanged(region);
}

void ColourPainter::erase(const QRegion &region)
{
    const QRegion paintable = paintableRegion(region);
    if (paintable.isEmpty())
        return;

    mColourLayer->erase(paintable.translated(-mColourLayer->position()));
    mMapDocument->emitRegionChanged(paintable);
}

QRegion ColourPainter::computeFillRegion(const QPoint &fillOrigin) const
{
    // Create that region that will hold the fill
    QRegion fillRegion;

    // Silently quit if parameters are unsatisfactory
    if (!isDrawable(fillOrigin.x(), fillOrigin.y()))
        return fillRegion;

    // Cache cell that we will match other cells against
    const QColor matchCell = cellAt(fillOrigin.x(), fillOrigin.y());

    // Grab map dimensions for later use.
    const int mapWidth = mMapDocument->map()->width();
    const int mapHeight = mMapDocument->map()->height();
    const int mapSize = mapWidth * mapHeight;

    // Create a queue to hold cells that need filling
    QList<QPoint> fillPositions;
    fillPositions.append(fillOrigin);

    // Create an array that will store which cells have been processed
    // This is faster than checking if a given cell is in the region/list
    QVector<quint8> processedCellsVec(mapSize);
    quint8 *processedCells = processedCellsVec.data();

    // Loop through queued positions and fill them, while at the same time
    // checking adjacent positions to see if they should be added
    while (!fillPositions.empty()) {
        const QPoint currentPoint = fillPositions.takeFirst();
        const int startOfLine = currentPoint.y() * mapWidth;

        // Seek as far left as we can
        int left = currentPoint.x();
        while (cellAt(left - 1, currentPoint.y()) == matchCell &&
               isDrawable(left - 1, currentPoint.y()))
            --left;

        // Seek as far right as we can
        int right = currentPoint.x();
        while (cellAt(right + 1, currentPoint.y()) == matchCell &&
               isDrawable(right + 1, currentPoint.y()))
            ++right;

        // Add cells between left and right to the region
        fillRegion += QRegion(left, currentPoint.y(), right - left + 1, 1);

        // Add cell strip to processed cells
        memset(&processedCells[startOfLine + left],
               1,
               right - left);

        // These variables cache whether the last cell was added to the queue
        // or not as an optimization, since adjacent cells on the x axis
        // do not need to be added to the queue.
        bool lastAboveCell = false;
        bool lastBelowCell = false;

        // Loop between left and right and check if cells above or
        // below need to be added to the queue
        for (int x = left; x <= right; ++x) {
            const QPoint fillPoint(x, currentPoint.y());

            // Check cell above
            if (fillPoint.y() > 0) {
                QPoint aboveCell(fillPoint.x(), fillPoint.y() - 1);
                if (!processedCells[aboveCell.y()*mapWidth + aboveCell.x()] &&
                    cellAt(aboveCell.x(), aboveCell.y()) == matchCell &&
                    isDrawable(aboveCell.x(), aboveCell.y()))
                {
                    // Do not add the above cell to the queue if its
                    // x-adjacent cell was added.
                    if (!lastAboveCell)
                        fillPositions.append(aboveCell);

                    lastAboveCell = true;
                } else lastAboveCell = false;

                processedCells[aboveCell.y() * mapWidth + aboveCell.x()] = 1;
            }

            // Check cell below
            if (fillPoint.y() + 1 < mapHeight) {
                QPoint belowCell(fillPoint.x(), fillPoint.y() + 1);
                if (!processedCells[belowCell.y()*mapWidth + belowCell.x()] &&
                    cellAt(belowCell.x(), belowCell.y()) == matchCell &&
                    isDrawable(belowCell.x(), belowCell.y()))
                {
                    // Do not add the below cell to the queue if its
                    // x-adjacent cell was added.
                    if (!lastBelowCell)
                        fillPositions.append(belowCell);

                    lastBelowCell = true;
                } else lastBelowCell = false;

                processedCells[belowCell.y() * mapWidth + belowCell.x()] = 1;
            }
        }
    }

    return fillRegion;
}

bool ColourPainter::isDrawable(int x, int y) const
{
    const QRegion &selection = mMapDocument->tileSelection();
    if (!(selection.isEmpty() || selection.contains(QPoint(x, y))))
        return false;

    const int layerX = x - mColourLayer->x();
    const int layerY = y - mColourLayer->y();

    if (!mColourLayer->contains(layerX, layerY))
        return false;

    return true;
}

QRegion ColourPainter::paintableRegion(const QRegion &region) const
{
    const QRegion bounds = QRegion(mColourLayer->bounds());
    QRegion intersection = bounds.intersected(region);

    const QRegion &selection = mMapDocument->tileSelection();
    if (!selection.isEmpty())
        intersection &= selection;

    return intersection;
}

