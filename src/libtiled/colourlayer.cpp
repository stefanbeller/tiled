#include "colourlayer.h"

#include <assert.h>

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tileset.h"

using namespace Tiled;

ColourLayer::ColourLayer(const QString &name,
                         int x, int y, int width, int height)
    : Layer(ColourLayerType, name, x, y, width, height)
    , mGrid(width * height)
{
    Q_ASSERT(width >= 0);
    Q_ASSERT(height >= 0);
}

ColourLayer::~ColourLayer()
{
}


void ColourLayer::resize(const QSize &size, const QPoint &offset)
{
    QVector<QColor> newGrid(size.width() * size.height());

    // Copy over the preserved part
    const int startX = qMax(0, -offset.x());
    const int startY = qMax(0, -offset.y());
    const int endX = qMin(mWidth, size.width() - offset.x());
    const int endY = qMin(mHeight, size.height() - offset.y());

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const int index = x + offset.x() + (y + offset.y()) * size.width();
            newGrid[index] = cellAt(x, y);
        }
    }

    mGrid = newGrid;

    Layer::resize(size, offset);
}

void ColourLayer::offset(const QPoint &offset,
                         const QRect &bounds,
                         bool wrapX, bool wrapY)
{
}

bool ColourLayer::canMergeWith(Layer *other) const
{
    return dynamic_cast<ColourLayer*>(other) != 0;
}

Layer *ColourLayer::mergedWith(Layer *other) const
{
    Q_ASSERT(canMergeWith(other));

    const ColourLayer *og = static_cast<ColourLayer*>(other);

    ColourLayer *merged = static_cast<ColourLayer*>(clone());
    return merged;
}

/**
 * Returns a duplicate of this ColourLayer.
 *
 * \sa Layer::clone()
 */
Layer *ColourLayer::clone() const
{
    return initializeClone(new ColourLayer(mName, mX, mY, mWidth, mHeight));
}

ColourLayer *ColourLayer::initializeClone(ColourLayer *clone) const
{
    Layer::initializeClone(clone);
    clone->mGrid = mGrid;
    assert(clone->mWidth*clone->mHeight == mGrid.size());
    return clone;
}

ColourLayer *ColourLayer::copy(const QRegion &region) const
{
    const QRegion area = region.intersected(QRect(0, 0, width(), height()));
    const QRect bounds = region.boundingRect();
    const QRect areaBounds = area.boundingRect();
    const int offsetX = qMax(0, areaBounds.x() - bounds.x());
    const int offsetY = qMax(0, areaBounds.y() - bounds.y());

    ColourLayer *copied = new ColourLayer(QString(),
                                      0, 0,
                                      bounds.width(), bounds.height());

    foreach (const QRect &rect, area.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                copied->setCell(x - areaBounds.x() + offsetX,
                                y - areaBounds.y() + offsetY,
                                cellAt(x, y));

    return copied;
}

bool ColourLayer::isEmpty() const
{
    for (int i = 0, i_end = mGrid.size(); i < i_end; ++i)
        if (mGrid.at(i).alpha() > 0)
            return false;

    return true;
}

QRegion ColourLayer::region() const
{
    QRegion region;

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            if (!cellAt(x, y).alpha() == 0) {
                const int rangeStart = x;
                for (++x; x <= mWidth; ++x) {
                    if (x == mWidth || cellAt(x, y).alpha()==0) {
                        const int rangeEnd = x;
                        region += QRect(rangeStart + mX, y + mY,
                                        rangeEnd - rangeStart, 1);
                        break;
                    }
                }
            }
        }
    }

    return region;
}

void ColourLayer::erase(const QRegion &area)
{
    const QColor emptyCell(0,0,0,0);
    foreach (const QRect &rect, area.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                setCell(x, y, emptyCell);
}

void ColourLayer::setCells(int x, int y, ColourLayer *layer,
                         const QRegion &mask)
{
    // Determine the overlapping area
    QRegion area = QRect(x, y, layer->width(), layer->height());
    area &= QRect(0, 0, width(), height());

    if (!mask.isEmpty())
        area &= mask;

    foreach (const QRect &rect, area.rects())
        for (int _x = rect.left(); _x <= rect.right(); ++_x)
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y)
                setCell(_x, _y, layer->cellAt(_x - x, _y - y));
}

void ColourLayer::merge(const QPoint &pos, const ColourLayer *layer)
{
    // Determine the overlapping area
    QRect area = QRect(pos, QSize(layer->width(), layer->height()));
    area &= QRect(0, 0, width(), height());

    for (int y = area.top(); y <= area.bottom(); ++y) {
        for (int x = area.left(); x <= area.right(); ++x) {
            const QColor &cell = layer->cellAt(x - area.left(),
                                             y - area.top());
            if (cell.alpha() > 0)
                setCell(x, y, cell);
        }
    }
}

void ColourLayer::flip(FlipDirection direction)
{
    QVector<QColor> newGrid(mWidth * mHeight);

    Q_ASSERT(direction == FlipHorizontally || direction == FlipVertically);

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            QColor &dest = newGrid[x + y * mWidth];
            if (direction == FlipHorizontally) {
                const QColor &source = cellAt(mWidth - x - 1, y);
                dest = source;
            } else if (direction == FlipVertically) {
                const QColor &source = cellAt(x, mHeight - y - 1);
                dest = source;
            }
        }
    }

    mGrid = newGrid;
}

void ColourLayer::rotate(RotateDirection direction)
{
    int newWidth = mHeight;
    int newHeight = mWidth;
    QVector<QColor> newGrid(newWidth * newHeight);

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            const QColor &source = cellAt(x, y);
            QColor dest = source;

            if (direction == RotateRight)
                newGrid[x * newWidth + (mHeight - y - 1)] = dest;
            else
                newGrid[(mWidth - x - 1) * newWidth + y] = dest;
        }
    }

    mWidth = newWidth;
    mHeight = newHeight;
    mGrid = newGrid;
}

void ColourLayer::fill(const QColor & colour)
{
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            setCell(x, y, colour);
        }
    }
}
