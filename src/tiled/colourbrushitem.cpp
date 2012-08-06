#include "colourbrushitem.h"

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "paintcolourlayer.h"
#include "tile.h"
#include "colourlayer.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

ColourBrushItem::ColourBrushItem():
    mMapDocument(0),
    mColourLayer(0)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}

void ColourBrushItem::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    // The tiles in the stamp may no longer be valid
    setColourLayer(0);
    updateBoundingRect();
}

void ColourBrushItem::setColourLayer(const Tiled::ColourLayer *colourLayer)
{
    delete mColourLayer;

    if (colourLayer) {
        mColourLayer = static_cast<Tiled::ColourLayer*>(colourLayer->clone());
        mRegion = mColourLayer->region();
    } else {
        mColourLayer = 0;
        mRegion = QRegion();
    }
    updateBoundingRect();
    update();
}

void ColourBrushItem::setColourLayerPosition(const QPoint &pos)
{
    if (!mColourLayer)
        return;

    const QPoint oldPosition(mColourLayer->x(), mColourLayer->y());
    if (oldPosition == pos)
        return;

    mRegion.translate(pos - oldPosition);
    mColourLayer->setX(pos.x());
    mColourLayer->setY(pos.y());
    updateBoundingRect();
}

void ColourBrushItem::setTileRegion(const QRegion &region)
{
    if (mRegion == region)
        return;

    mRegion = region;
    updateBoundingRect();
}

QRectF ColourBrushItem::boundingRect() const
{
    return mBoundingRect;
}

void ColourBrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *)
{
    QColor highlight = QApplication::palette().highlight().color();
    highlight.setAlpha(64);

    const MapRenderer *renderer = mMapDocument->renderer();

    if (mColourLayer) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        renderer->drawColourLayer(painter, mColourLayer, option->exposedRect);
        painter->setOpacity(opacity);

        renderer->drawTileSelection(painter, mRegion, highlight,
                                    option->exposedRect);
    } else {
        renderer->drawTileSelection(painter, mRegion, highlight,
                                    option->exposedRect);
    }
}

void ColourBrushItem::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mMapDocument) {
        mBoundingRect = QRectF();
        return;
    }

    const QRect bounds = mRegion.boundingRect();
    mBoundingRect = mMapDocument->renderer()->boundingRect(bounds);
}

