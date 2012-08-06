#ifndef COLOURLAYERITEM_H
#define COLOURLAYERITEM_H

#include <QGraphicsItem>

namespace Tiled {

class MapRenderer;
class ColourLayer;

namespace Internal {

/**
 * A graphics item displaying a vertex colour layer in a QGraphicsView.
 */
class ColourLayerItem : public QGraphicsItem
{
public:
    /**
     * Constructor.
     *
     * @param layer    the tile layer to be displayed
     * @param renderer the map renderer to use to render the layer
     */
    ColourLayerItem(ColourLayer *layer, MapRenderer *renderer);

    /**
     * Updates the size and position of this item. Should be called when the
     * size of either the tile layer or its associated map have changed.
     *
     * Calling this function when the size of the map changes is necessary
     * because in certain map orientations this affects the layer position
     * (when using the IsometricRenderer for example).
     */
    void syncWithColourLayer();

    // QGraphicsItem
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    ColourLayer *mLayer;
    MapRenderer *mRenderer;
    QRectF mBoundingRect;
};

} // namespace Internal
} // namespace Tiled

#endif // COLOURLAYERITEM_H
