#include "paintcolourlayer.h"

#include "map.h"
#include "mapdocument.h"
#include "colourlayer.h"
#include "colourpainter.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

PaintColourLayer::PaintColourLayer(MapDocument *mapDocument,
                               ColourLayer *target,
                               int x,
                               int y,
                               const ColourLayer *source):
    mMapDocument(mapDocument),
    mTarget(target),
    mSource(static_cast<ColourLayer*>(source->clone())),
    mX(x),
    mY(y),
    mPaintedRegion(x, y, source->width(), source->height()),
    mMergeable(false)
{
    mErased = mTarget->copy(mX - mTarget->x(),
                            mY - mTarget->y(),
                            mSource->width(), mSource->height());
    setText(QCoreApplication::translate("Undo Commands", "Paint"));
}

PaintColourLayer::~PaintColourLayer()
{
    delete mSource;
    delete mErased;
}

void PaintColourLayer::undo()
{
    ColourPainter painter(mMapDocument, mTarget);
    painter.setCells(mX, mY, mErased, mPaintedRegion);
}

void PaintColourLayer::redo()
{
    ColourPainter painter(mMapDocument, mTarget);
    painter.drawCells(mX, mY, mSource);
}

bool PaintColourLayer::mergeWith(const QUndoCommand *other)
{
    const PaintColourLayer *o = static_cast<const PaintColourLayer*>(other);
    if (!(mMapDocument == o->mMapDocument &&
          mTarget == o->mTarget &&
          o->mMergeable))
        return false;

    const QRegion newRegion = o->mPaintedRegion.subtracted(mPaintedRegion);
    const QRegion combinedRegion = mPaintedRegion.united(o->mPaintedRegion);
    const QRect bounds = QRect(mX, mY, mSource->width(), mSource->height());
    const QRect combinedBounds = combinedRegion.boundingRect();

    // Resize the erased tiles and source layers when necessary
    if (bounds != combinedBounds) {
        const QPoint shift = bounds.topLeft() - combinedBounds.topLeft();
        mErased->resize(combinedBounds.size(), shift);
        mSource->resize(combinedBounds.size(), shift);
    }

    mX = combinedBounds.left();
    mY = combinedBounds.top();
    mPaintedRegion = combinedRegion;

    // Copy the painted tiles from the other command over
    const QPoint pos = QPoint(o->mX, o->mY) - combinedBounds.topLeft();
    mSource->merge(pos, o->mSource);

    // Copy the newly erased tiles from the other command over
    foreach (const QRect &rect, newRegion.rects())
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                mErased->setCell(x - mX,
                                 y - mY,
                                 o->mErased->cellAt(x - o->mX, y - o->mY));

    return true;
}

