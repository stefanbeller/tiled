#ifndef SMARTMAGIC_H
#define SMARTMAGIC_H

#include <QObject>
#include <QRegion>

#include "mapanalyzer.h"

namespace Tiled {

class Cell;
class Layer;
class Map;
class TileLayer;
class Tileset;

namespace Internal {
class MapDocument;

class SmartMagic : public QObject
{
    Q_OBJECT
public:
    SmartMagic();
    ~SmartMagic();

    void setMapDocument(MapDocument *mapDocument);
    void setMapAnalyzer(MapAnalyzer *mapAnalyzer);

public slots:
    void applyMagic(QRegion r, Layer* where);

private:
    MapDocument *mMapDocument;
    MapAnalyzer *mMapAnalyzer;
};

} // namespace Internal
} // namespace Tiled

#endif // SMARTMAGIC_H
