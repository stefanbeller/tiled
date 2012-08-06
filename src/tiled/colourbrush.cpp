#include "colourbrush.h"

#include "colourbrushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "paintcolourlayer.h"
#include "colourlayer.h"

#include <math.h>
#include <QVector>

using namespace Tiled;
using namespace Tiled::Internal;

ColourBrush::ColourBrush(QObject *parent)
    : AbstractColourTool(tr("Colour Brush"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-clone.png")),
                       QKeySequence(tr("B")),
                       parent)
    , mStamp(0)
    , mStampX(0), mStampY(0)
    , mBrushBehavior(Free)
    , mStampReferenceX(0)
    , mStampReferenceY(0)
    , mIsRandom(false)
{
}

ColourBrush::~ColourBrush()
{
    delete mStamp;
}


/**
 * Returns a lists of points on an ellipse.
 * (x0,y0) is the midpoint
 * (x1,y1) to determines the radius.
 * It is adapted from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 * here is the orginal: http://homepage.smc.edu/kennedy_john/belipse.pdf
 */
static QVector<QPoint> rasterEllipse(int x0, int y0, int x1, int y1)
{
    QVector<QPoint> ret;
    int x, y;
    int xChange, yChange;
    int ellipseError;
    int twoXSquare, twoYSquare;
    int stoppingX, stoppingY;
    int radiusX = x0 > x1 ? x0 - x1 : x1 - x0;
    int radiusY = y0 > y1 ? y0 - y1 : y1 - y0;

    if (radiusX == 0 && radiusY == 0)
        return ret;

    twoXSquare = 2 * radiusX * radiusX;
    twoYSquare = 2 * radiusY * radiusY;
    x = radiusX;
    y = 0;
    xChange = radiusY * radiusY * (1 - 2 * radiusX);
    yChange = radiusX * radiusX;
    ellipseError = 0;
    stoppingX = twoYSquare*radiusX;
    stoppingY = 0;
    while ( stoppingX >= stoppingY ) {
        ret += QPoint(x0 + x, y0 + y);
        ret += QPoint(x0 - x, y0 + y);
        ret += QPoint(x0 + x, y0 - y);
        ret += QPoint(x0 - x, y0 - y);
        y++;
        stoppingY += twoXSquare;
        ellipseError += yChange;
        yChange += twoXSquare;
        if ((2 * ellipseError + xChange) > 0 ) {
            x--;
            stoppingX -= twoYSquare;
            ellipseError += xChange;
            xChange += twoYSquare;
        }
    }
    x = 0;
    y = radiusY;
    xChange = radiusY * radiusY;
    yChange = radiusX * radiusX * (1 - 2 * radiusY);
    ellipseError = 0;
    stoppingX = 0;
    stoppingY = twoXSquare * radiusY;
    while ( stoppingX <= stoppingY ) {
        ret += QPoint(x0 + x, y0 + y);
        ret += QPoint(x0 - x, y0 + y);
        ret += QPoint(x0 + x, y0 - y);
        ret += QPoint(x0 - x, y0 - y);
        x++;
        stoppingX += twoYSquare;
        ellipseError += xChange;
        xChange += twoYSquare;
        if ((2 * ellipseError + yChange) > 0 ) {
            y--;
            stoppingY -= twoXSquare;
            ellipseError += yChange;
            yChange += twoXSquare;
        }
    }

    return ret;
}

/**
 * Returns the lists of points on a line from (x0,y0) to (x1,y1).
 *
 * This is an implementation of bresenhams line algorithm, initially copied
 * from http://en.wikipedia.org/wiki/Bresenham's_line_algorithm#Optimization
 * changed to C++ syntax.
 */
static QVector<QPoint> calculateLine(int x0, int y0, int x1, int y1)
{
    QVector<QPoint> ret;

    bool steep = qAbs(y1 - y0) > qAbs(x1 - x0);
    if (steep) {
        qSwap(x0, y0);
        qSwap(x1, y1);
    }
    if (x0 > x1) {
        qSwap(x0, x1);
        qSwap(y0, y1);
    }
    const int deltax = x1 - x0;
    const int deltay = qAbs(y1 - y0);
    int error = deltax / 2;
    int ystep;
    int y = y0;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (int x = x0; x < x1 + 1 ; x++) {
        if (steep)
            ret += QPoint(y, x);
        else
            ret += QPoint(x, y);
        error = error - deltay;
        if (error < 0) {
             y = y + ystep;
             error = error + deltax;
        }
    }

    return ret;
}

void ColourBrush::tilePositionChanged(const QPoint &)
{
    const int x = mStampX;
    const int y = mStampY;
    updatePosition();
    switch (mBrushBehavior) {
    case Paint:
        foreach (const QPoint &p, calculateLine(x, y, mStampX, mStampY))
            doPaint(true, p.x(), p.y());
        break;
    case LineStartSet:
        configureBrush(calculateLine(mStampReferenceX, mStampReferenceY,
                                     mStampX, mStampY));
        break;
    case CircleMidSet:
        configureBrush(rasterEllipse(mStampReferenceX, mStampReferenceY,
                                     mStampX, mStampY));
        break;
    case Capture:
        brushItem()->setTileRegion(capturedArea());
        break;
    case Line:
    case Circle:
        updatePosition();
        break;
    case Free:
        updatePosition();
        break;
    }
}

void ColourBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (!brushItem()->isVisible())
        return;

    if (event->button() == Qt::LeftButton) {
        switch (mBrushBehavior) {
        case Line:
            mStampReferenceX = mStampX;
            mStampReferenceY = mStampY;
            mBrushBehavior = LineStartSet;
            break;
        case Circle:
            mStampReferenceX = mStampX;
            mStampReferenceY = mStampY;
            mBrushBehavior = CircleMidSet;
            break;
        case LineStartSet:
            doPaint(false, 0, 0);
            mStampReferenceX = mStampX;
            mStampReferenceY = mStampY;
            break;
        case CircleMidSet:
            doPaint(false, 0, 0);
            break;
        case Paint:
            beginPaint();
            break;
        case Free:
            beginPaint();
            mBrushBehavior = Paint;
            break;
        case Capture:
            break;
        }
    } else {
        if (event->button() == Qt::RightButton)
            beginCapture();
    }
}

void ColourBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
    case Capture:
        if (event->button() == Qt::RightButton) {
            endCapture();
            mBrushBehavior = Free;
        }
        break;
    case Paint:
        if (event->button() == Qt::LeftButton)
            mBrushBehavior = Free;
    default:
        // do nothing?
        break;
    }
}

void ColourBrush::configureBrush(const QVector<QPoint> &list)
{
    if (!mStamp)
        return;

    QRegion reg;
    QRegion stampRegion;

    if (mIsRandom)
        stampRegion = brushItem()->colourLayer()->region();
    else
        stampRegion = mStamp->region();

    Map *map = mapDocument()->map();

    ColourLayer *stamp = new ColourLayer(QString(), 0, 0,
                                     map->width(), map->height());

    foreach (const QPoint p, list) {
        const QRegion update = stampRegion.translated(p.x() - mStampX,
                                                      p.y() - mStampY);
        if (!reg.intersects(update)) {
            reg += update;

            if (mIsRandom) {
                ColourLayer *newStamp = getRandomColourLayer();
                stamp->merge(p, newStamp);
                delete newStamp;
            } else {
                stamp->merge(p, mStamp);
            }

        }
    }

    brushItem()->setColourLayer(stamp);
    delete stamp;
}

void ColourBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (!mStamp)
        return;

    if (modifiers & Qt::ShiftModifier) {
        mBrushBehavior = Line;
        if (modifiers & Qt::ControlModifier) {
            mBrushBehavior = Circle;
            // while finding the mid point, there is no need to show
            // the (maybe bigger than 1x1) stamp
            brushItem()->setColourLayer(0);
            brushItem()->setTileRegion(QRect(tilePosition(), QSize(1, 1)));
        }
    } else {
        mBrushBehavior = Free;
    }

    switch (mBrushBehavior) {
    case Circle:
        // do not update brushItems ColourLayer by setStamp
        break;
    default:
        if (mIsRandom)
            setRandomStamp();
        else
            brushItem()->setColourLayer(mStamp);

        updatePosition();
    }
}

void ColourBrush::languageChanged()
{
    setName(tr("Stamp Brush"));
    setShortcut(QKeySequence(tr("B")));
}

void ColourBrush::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    AbstractColourTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->setTileRegion(QRegion());
    setStamp(0);
}

ColourLayer *ColourBrush::getRandomColourLayer() const
{
    if (mRandomList.empty())
        return 0;

    ColourLayer *ret = new ColourLayer(QString(), 0, 0, 1, 1);
    ret->setCell(0, 0, mRandomList.at(rand() % mRandomList.size()));
    return ret;
}

void ColourBrush::updateRandomList()
{
    if (!mStamp)
        return;

    mRandomList.clear();
    for (int x = 0; x < mStamp->width(); x++)
        for (int y = 0; y < mStamp->height(); y++)
            if (mStamp->cellAt(x,y).alpha() != 0)
                mRandomList.append(mStamp->cellAt(x,y));
}

void ColourBrush::setStamp(ColourLayer *stamp)
{
    if (mStamp == stamp)
        return;

    delete mStamp;
    mStamp = stamp;

    if (mIsRandom) {
        updateRandomList();
        setRandomStamp();
    } else {
        brushItem()->setColourLayer(mStamp);
    }

    updatePosition();
}

void ColourBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false, mStampX, mStampY);
}

void ColourBrush::beginCapture()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Capture;

    mCaptureStart = tilePosition();

    setStamp(0);
}

void ColourBrush::endCapture()
{
    if (mBrushBehavior != Capture)
        return;

    mBrushBehavior = Free;

    ColourLayer *colourLayer = currentColourLayer();
    Q_ASSERT(colourLayer);

    // Intersect with the layer and translate to layer coordinates
    QRect captured = capturedArea();
    captured.intersect(QRect(colourLayer->x(), colourLayer->y(),
                             colourLayer->width(), colourLayer->height()));

    if (captured.isValid()) {
        captured.translate(-colourLayer->x(), -colourLayer->y());
        ColourLayer *capture = colourLayer->copy(captured);
        emit currentTilesChanged(capture);
        // A copy will have been created, so delete this version
        delete capture;
    } else {
        updatePosition();
    }
}

QRect ColourBrush::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, tilePosition()).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

void ColourBrush::doPaint(bool mergeable, int whereX, int whereY)
{
    ColourLayer *stamp = brushItem()->colourLayer();

    if (!stamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    ColourLayer *colourLayer = currentColourLayer();
    Q_ASSERT(colourLayer);

    if (!colourLayer->bounds().intersects(QRect(whereX, whereY,
                                              stamp->width(),
                                              stamp->height())))
        return;

    PaintColourLayer *paint = new PaintColourLayer(mapDocument(), colourLayer,
                            whereX, whereY, stamp);
    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    mapDocument()->emitRegionEdited(brushItem()->tileRegion(), colourLayer);
}

/**
 * Updates the position of the brush item.
 */
void ColourBrush::updatePosition()
{
    if (mIsRandom)
        setRandomStamp();

    const QPoint tilePos = tilePosition();

    if (!brushItem()->colourLayer()) {
        brushItem()->setTileRegion(QRect(tilePos, QSize(1, 1)));
        mStampX = tilePos.x();
        mStampY = tilePos.y();
    }

    if (mIsRandom || !mStamp) {
        mStampX = tilePos.x();
        mStampY = tilePos.y();
    } else {
        mStampX = tilePos.x() - mStamp->width() / 2;
        mStampY = tilePos.y() - mStamp->height() / 2;
    }
    brushItem()->setColourLayerPosition(QPoint(mStampX, mStampY));
}

void ColourBrush::setRandom(bool value)
{
    mIsRandom = value;

    if (mIsRandom) {
        updateRandomList();
        setRandomStamp();
    } else {
        brushItem()->setColourLayer(mStamp);
    }
}

void ColourBrush::setRandomStamp()
{
    ColourLayer *t = getRandomColourLayer();
    brushItem()->setColourLayer(t);
    delete t;
}

