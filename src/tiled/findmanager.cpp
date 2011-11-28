#include "findmanager.h"

#include <QUndoStack>

#include "changetileselection.h"
#include "documentmanager.h"
#include "utils.h"

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;

FindManager *FindManager::mInstance = 0;

FindManager *FindManager::instance()
{
    if (!mInstance)
        mInstance = new FindManager;
    return mInstance;
}

void FindManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

FindManager::FindManager()
{
}

FindManager::~FindManager()
{
}

QList<QRegion> FindManager::getFindResults()
{

}

void FindManager::markDifference()
{
    QList<QRegion> regions = getFindResults();

    if (regions.size() == 0)
        return;
    else if (mCurrentResult < 0)
        mCurrentResult = regions.size() - 1;
    else if (mCurrentResult >= regions.size())
        mCurrentResult = 0;
    QRegion diff = regions.at(mCurrentResult);

    Q_ASSERT (!diff.isEmpty());

    QPoint mid = getMidPoint(diff);
    DocumentManager::instance()->centerViewOn(mid.x(), mid.y());
    QUndoStack *stack = mMapDocument->undoStack();
    stack->push(new ChangeTileSelection(mMapDocument, diff));
}

void FindManager::markPrevious()
{
    mCurrentResult--;
    markDifference();
}

void FindManager::markNext()
{
    mCurrentResult++;
    markDifference();
}
