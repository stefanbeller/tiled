#ifndef COLOURLAYER_H
#define COLOURLAYER_H

#include "tiled_global.h"

#include "layer.h"

#include <QColor>
#include <QMargins>

namespace Tiled {

class MapObject;

/**
 * A map that purely stores a single colour value for each tile
 */
class TILEDSHARED_EXPORT ColourLayer : public Layer
{
public:
    enum FlipDirection {
        FlipHorizontally,
        FlipVertically,
        FlipDiagonally
    };

    enum RotateDirection {
        RotateLeft,
        RotateRight
    };


    /**
     * Constructor with some parameters.
     */
    ColourLayer(const QString &name, int x, int y, int width, int height);

    /**
     * Destructor.
     */
    ~ColourLayer();

    /**
     * Computes and returns the set of tilesets used by this layer.
     */
    virtual QSet<Tileset*> usedTilesets() const { return QSet<Tileset*>(); }

    /**
     * Returns whether (x, y) is inside this map layer.
     */
    bool contains(int x, int y) const
    { return x >= 0 && y >= 0 && x < mWidth && y < mHeight; }

    bool contains(const QPoint &point) const
    { return contains(point.x(), point.y()); }

    /**
     * Resizes this object group to \a size, while shifting all objects by
     * \a offset tiles.
     *
     * \sa Layer::resize()
     */
    virtual void resize(const QSize &size, const QPoint &offset);

    /**
     * Calculates the region occupied by the tiles of this layer. Similar to
     * Layer::bounds(), but leaves out the regions without tiles.
     */
    QRegion region() const;

    /**
     * Offsets all objects within the group, and optionally wraps them. The
     * object's center must be within \a bounds, and wrapping occurs if the
     * displaced center is out of the bounds.
     *
     * \sa Layer::offset()
     */
    virtual void offset(const QPoint &offset, const QRect &bounds,
                        bool wrapX, bool wrapY);

    /**
     * Returns whether this tile layer is referencing the given tileset.
     */
    bool referencesTileset(const Tileset * /*tileset*/ ) const { return false; }

    /**
     * Returns the region of tiles coming from the given \a tileset.
     */
    QRegion tilesetReferences(Tileset * /* tileset */) const { return QRegion(); }

    /**
     * Removes all references to the given tileset. This sets all tiles on this
     * layer that are from the given tileset to null.
     */
    void removeReferencesToTileset(Tileset * /* tileset */) {}

    /**
     * Replaces all tiles from \a oldTileset with tiles from \a newTileset.
     */
    void replaceReferencesToTileset(Tileset * /*oldTileset*/, Tileset * /*newTileset*/) {}

    /**
     * Returns a read-only reference to the cell at the given coordinates. The
     * coordinates have to be within this layer.
     */
    const QColor &cellAt(int x, int y) const
    { return mGrid.at(x + y * mWidth); }

    const QColor &cellAt(const QPoint &point) const
    { return cellAt(point.x(), point.y()); }

    /**
     * Sets the cell at the given coordinates.
     */
    void setCell(int x, int y, const QColor &cell)
    { mGrid[x + y * mWidth] = cell; }

    /**
     * Removes all cells in the specified region.
     */
    void erase(const QRegion &region);

    /**
     * Returns a copy of the area specified by the given \a region. The
     * caller is responsible for the returned tile layer.
     */
    ColourLayer *copy(const QRegion &region) const;

    ColourLayer *copy(int x, int y, int width, int height) const
    { return copy(QRegion(x, y, width, height)); }

    /**
     * Merges the given \a layer onto this layer at position \a pos. Parts that
     * fall outside of this layer will be lost and empty tiles in the given
     * layer will have no effect.
     */
    void merge(const QPoint &pos, const ColourLayer *layer);

    /**
     * Sets the cells starting at the given position to the cells in the given
     * \a colourLayer. Parts that fall outside of this layer will be ignored.
     *
     * When a \a mask is given, only cells that fall within this mask are set.
     * The mask is applied in local coordinates.
     */
    void setCells(int x, int y, ColourLayer *colourLayer,
                  const QRegion &mask = QRegion());

    bool canMergeWith(Layer *other) const;
    Layer *mergedWith(Layer *other) const;

    /**
     * Returns true if all tiles in the layer are empty.
     */
    bool isEmpty() const;

    Layer *clone() const;

    virtual ColourLayer *asColourLayer() { return this; }

    /**
     * Flip this tile layer in the given \a direction. Direction must be
     * horizontal or vertical. This doesn't change the dimensions of the
     * tile layer.
     */
    void flip(FlipDirection direction);

    /**
     * Rotate this tile layer by 90 degrees left or right. The tile positions
     * are rotated within the layer, and the tiles themselves are rotated. The
     * dimensions of the tile layer are swapped.
     */
    void rotate(RotateDirection direction);


    void fill(const QColor & colour);
protected:
    ColourLayer *initializeClone(ColourLayer *clone) const;

private:
    QVector<QColor> mGrid;
};

} // namespace Tiled

#endif // COLOURLAYER_H
