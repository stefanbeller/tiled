#ifndef ABSTRACTCOLOURTOOL_H
#define ABSTRACTCOLOURTOOL_H

#include "abstracttool.h"

namespace Tiled {

class ColourLayer;

namespace Internal {

class ColourBrushItem;
class MapDocument;

/**
 * A convenient base class for colour based tools.
 */
class AbstractColourTool : public AbstractTool
{
    Q_OBJECT

public:
    /**
     * Constructs an abstract tile tool with the given \a name and \a icon.
     */
    AbstractColourTool(const QString &name,
                     const QIcon &icon,
                     const QKeySequence &shortcut,
                     QObject *parent = 0);

    ~AbstractColourTool();

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);

    void mouseEntered();
    void mouseLeft();
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);

protected:
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);

    /**
     * Overridden to only enable this tool when the currently selected layer is
     * a colour layer.
     */
    void updateEnabledState();

    /**
     * New virtual method to implement for tile tools. This method is called
     * on mouse move events, but only when the tile position changes.
     */
    virtual void tilePositionChanged(const QPoint &tilePos) = 0;

    /**
     * Updates the status info with the current tile position. When the mouse
     * is not in the view, the status info is set to an empty string.
     *
     * This behaviour can be overridden in a subclass. This method is
     * automatically called after each call to tilePositionChanged() and when
     * the brush visibility changes.
     */
    virtual void updateStatusInfo();

    bool isBrushVisible() const { return mBrushVisible; }

    /**
     * Determines what the tile position means.
     */
    enum TilePositionMethod {
        OnTiles,       /**< Tile position is the tile the mouse is on. */
        BetweenTiles   /**< Tile position is between the tiles. */
    };

    void setTilePositionMethod(TilePositionMethod method)
    { mTilePositionMethod = method; }

    /**
     * Returns the last recorded tile position of the mouse.
     */
    QPoint tilePosition() const { return QPoint(mTileX, mTileY); }

    /**
     * Returns the brush item. The brush item is used to give an indication of
     * what a tile tool is going to do when used. It is automatically shown or
     * hidden based on whether the mouse is in the scene and whether the
     * currently selected layer is a colour layer.
     */
    ColourBrushItem *brushItem() const { return mBrushItem; }

    /**
     * Returns the current colour layer, or 0 if no colour layer is currently
     * selected.
     */
    ColourLayer *currentColourLayer() const;

private:
    void setBrushVisible(bool visible);
    void updateBrushVisibility();

    TilePositionMethod mTilePositionMethod;
    ColourBrushItem *mBrushItem;
    int mTileX, mTileY;
    bool mBrushVisible;
};

} // namespace Internal
} // namespace Tiled

#endif // ABSTRACTCOLOURTOOL_H
