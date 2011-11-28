#ifndef FINDMANAGER_H
#define FINDMANAGER_H

#include <QList>
#include <QRegion>

namespace Tiled {
class Map;

namespace Internal {
class MapDocument;

class FindManager
{
public:
    static FindManager *instance();
    static void deleteInstance();

    void setSearchPattern(Map *searchPattern);
    void setMapDocument(MapDocument *mapDocument);

    void markNext();
    void markPrevious();

    // could be private
    void markDifference();
    QList<QRegion> getFindResults();

private:
    FindManager();
    ~FindManager();
    static FindManager *mInstance;

    Map *mSearchPattern;
    MapDocument *mMapDocument;

    int mCurrentResult;
};

} // namespace Internal
} // namespace Tiled

#endif // FINDMANAGER_H
