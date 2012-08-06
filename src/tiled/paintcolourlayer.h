#ifndef PAINTCOLOURLAYER_H
#define PAINTCOLOURLAYER_H

#include "undocommands.h"

#include <QRegion>
#include <QUndoCommand>

namespace Tiled {

class ColourLayer;

namespace Internal {

class MapDocument;

/**
 * A command that paints one tile layer on top of another tile layer.
 */
class PaintColourLayer : public QUndoCommand
{
public:
    /**
     * Constructor.
     *
     * @param mapDocument the map document that's being edited
     * @param target      the target layer to paint on
     * @param x           the x position of the paint location
     * @param y           the y position of the paint location
     * @param source      the source layer to paint on the target layer
     */
    PaintColourLayer(MapDocument *mapDocument,
                   ColourLayer *target,
                   int x, int y,
                   const ColourLayer *source);

    ~PaintColourLayer();

    /**
     * Sets whether this undo command can be merged with an existing command.
     */
    void setMergeable(bool mergeable)
    { mMergeable = mergeable; }

    void undo();
    void redo();

    int id() const { return Cmd_PaintColourLayer; }
    bool mergeWith(const QUndoCommand *other);

private:
    MapDocument *mMapDocument;
    ColourLayer *mTarget;
    ColourLayer *mSource;
    ColourLayer *mErased;
    int mX, mY;
    QRegion mPaintedRegion;
    bool mMergeable;
};

} // namespace Internal
} // namespace Tiled

#endif // PAINTCOLOURLAYER_H

