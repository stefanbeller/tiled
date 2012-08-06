
#include "colourlayeritem.h"

#include "map.h"
#include "maprenderer.h"
#include "colourlayer.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;
using namespace Tiled::Internal;

ColourLayerItem::ColourLayerItem(ColourLayer *layer, MapRenderer *renderer)
    : mLayer(layer)
    , mRenderer(renderer)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

    syncWithColourLayer();
    setOpacity(mLayer->opacity());
}

void ColourLayerItem::syncWithColourLayer()
{
    prepareGeometryChange();
    mBoundingRect = mRenderer->boundingRect(mLayer->bounds());
}

QRectF ColourLayerItem::boundingRect() const
{
    return mBoundingRect;
}

void ColourLayerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *)
{
    // TODO: Display a border around the layer when selected
    mRenderer->drawColourLayer(painter, mLayer, option->exposedRect);
}

