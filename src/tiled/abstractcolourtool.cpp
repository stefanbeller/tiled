#include "abstracecolourtool.h"

#include "colourbrushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "colourlayer.h"

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

AbstractColourTool::AbstractColourTool(const QString &name,
                                   const QIcon &icon,
                                   const QKeySequence &shortcut,
                                   QObject *parent)
    : AbstractTool(name, icon, shortcut, parent)
    , mTilePositionMethod(OnTiles)
    , mBrushItem(new ColourBrushItem)
    , mTileX(0), mTileY(0)
    , mBrushVisible(false)
{
    mBrushItem->setVisible(false);
    mBrushItem->setZValue(10000);
}

AbstractColourTool::~AbstractColourTool()
{
    delete mBrushItem;
}

void AbstractColourTool::activate(MapScene *scene)
{
    scene->addItem(mBrushItem);
}

void AbstractColourTool::deactivate(MapScene *scene)
{
    scene->removeItem(mBrushItem);
}

void AbstractColourTool::mouseEntered()
{
    setBrushVisible(true);
}

void AbstractColourTool::mouseLeft()
{
    setBrushVisible(false);
}

void AbstractColourTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePosF = renderer->pixelToTileCoords(pos);
    QPoint tilePos;

    if (mTilePositionMethod == BetweenTiles)
        tilePos = tilePosF.toPoint();
    else
        tilePos = QPoint((int) std::floor(tilePosF.x()),
                         (int) std::floor(tilePosF.y()));

    if (mTileX != tilePos.x() || mTileY != tilePos.y()) {
        mTileX = tilePos.x();
        mTileY = tilePos.y();

        tilePositionChanged(tilePos);
        updateStatusInfo();
    }
}

void AbstractColourTool::mapDocumentChanged(MapDocument *oldDocument,
                                          MapDocument *newDocument)
{
    Q_UNUSED(oldDocument)
    mBrushItem->setMapDocument(newDocument);
}

void AbstractColourTool::updateEnabledState()
{
    setEnabled(currentColourLayer() != 0);
}

void AbstractColourTool::updateStatusInfo()
{
    if (mBrushVisible) {
        setStatusInfo(QString(QLatin1String("%1, %2"))
                      .arg(mTileX).arg(mTileY));
    } else {
        setStatusInfo(QString());
    }
}

void AbstractColourTool::setBrushVisible(bool visible)
{
    if (mBrushVisible == visible)
        return;

    mBrushVisible = visible;
    updateStatusInfo();
    updateBrushVisibility();
}

void AbstractColourTool::updateBrushVisibility()
{
    // Show the tile brush only when a visible tile layer is selected
    bool showBrush = false;
    if (mBrushVisible) {
        if (Layer *layer = currentColourLayer()) {
            if (layer->isVisible())
                showBrush = true;
        }
    }
    mBrushItem->setVisible(showBrush);
}

ColourLayer *AbstractColourTool::currentColourLayer() const
{
    if (!mapDocument())
        return 0;

    return dynamic_cast<ColourLayer*>(mapDocument()->currentLayer());
}

