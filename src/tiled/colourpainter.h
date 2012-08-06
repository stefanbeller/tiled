#ifndef COLOURPAINTER_H
#define COLOURPAINTER_H

#include <QRegion>

namespace Tiled {

class ColourLayer;

namespace Internal {

class MapDocument;

/**
 * The tile painter is meant for painting cells of a tile layer. It makes sure
 * that each paint operation sends out the proper events, so that views can
 * redraw the changed parts.
 *
 * This class also does bounds checking and when there is a tile selection, it
 * will only draw within this selection.
 */
class ColourPainter
{
public:
    /**
     * Constructs a tile painter.
     *
     * @param mapDocument the map document to send change notifications to
     * @param ColourLayer   the tile layer to edit
     */
    ColourPainter(MapDocument *mapDocument, ColourLayer *colourLayer);

    /**
     * Returns the cell at the given coordinates. The coordinates are relative
     * to the map origin. Returns an empty cell if the coordinates lay outside
     * of the layer.
     */
    QColor cellAt(int x, int y) const;

    /**
     * Sets the cell at the given coordinates. The coordinates are relative to
     * the map origin.
     */
    void setCell(int x, int y, const QColor &cell);

    /**
     * Sets the cells at the given coordinates to the cells in the given tile
     * layer. The coordinates \a x and \a y are relative to the map origin.
     *
     * When a \a mask is given, only cells that fall within this mask are set.
     * The mask is applied in map coordinates.
     */
    void setCells(int x, int y, ColourLayer *colourLayer,
                  const QRegion &mask = QRegion());

    /**
     * Draws the cells in the given tile layer at the given coordinates. The
     * coordinates \a x and \a y are relative to the map origin.
     *
     * Empty cells are skipped.
     */
    void drawCells(int x, int y, ColourLayer *colourLayer);

    /**
     * Draws the stamp within the given \a drawRegion region, repeating the
     * stamp as needed.
     */
    void drawStamp(const ColourLayer *stamp, const QRegion &drawRegion);

    /**
     * Erases the cells in the given region.
     */
    void erase(const QRegion &region);

    /**
     * Computes a fill region made up of all cells of the same type as that
     * at \a fillOrigin that are connected.
     */
    QRegion computeFillRegion(const QPoint &fillOrigin) const;

    /**
     * Returns true if the given cell is drawable.
     */
    bool isDrawable(int x, int y) const;

private:
    QRegion paintableRegion(const QRegion &region) const;
    QRegion paintableRegion(int x, int y, int width, int height) const
    { return paintableRegion(QRect(x, y, width, height)); }

    MapDocument *mMapDocument;
    ColourLayer *mColourLayer;
};

} // namespace Tiled
} // namespace Internal

#endif // COLOURPAINTER_H
