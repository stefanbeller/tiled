#ifndef COLOURBRUSHITEM_H
#define COLOURBRUSHITEM_H

#include <QGraphicsItem>

namespace Tiled {

class ColourLayer;

namespace Internal {

class MapDocument;

/**
 * This brush item is used to represent a brush in a map scene before it is
 * used.
 */
class ColourBrushItem : public QGraphicsItem
{
public:
    /**
     * Constructor.
     */
    ColourBrushItem();

    /**
     * Sets the map document this brush is operating on.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Sets a tile layer representing this brush. When no tile layer is set,
     * the brush only draws the selection color.
     *
     * The BrushItem does not take ownership over the tile layer, but makes a
     * personal copy of the tile layer.
     */
    void setColourLayer(const ColourLayer *colourLayer);

    /**
     * Returns the current tile layer.
     */
    ColourLayer *colourLayer() const { return mColourLayer; }

    /**
     * Changes the position of the tile layer, if one is set.
     */
    void setColourLayerPosition(const QPoint &pos);

    /**
     * Sets the region of tiles that this brush item occupies.
     */
    void setTileRegion(const QRegion &region);

    /**
     * Returns the region of the current tile layer or the region that was set
     * using setTileRegion.
     */
    QRegion tileRegion() const { return mRegion; }

    // QGraphicsItem
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    void updateBoundingRect();

    MapDocument *mMapDocument;
    Tiled::ColourLayer *mColourLayer;
    QRegion mRegion;
    QRectF mBoundingRect;
};

} // namespace Internal
} // namespace Tiled

#endif // COLOURBRUSHITEM_H
