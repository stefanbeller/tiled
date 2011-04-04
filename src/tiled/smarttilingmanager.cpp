/*
 * smarttilingmanager.cpp
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "smarttilingmanager.h"

#include "map.h"
#include "tile.h"
#include "tileset.h"

#include <QDebug>
#include <QImage>
#include <QList>
#include <QPixmap>
#include <QPoint>

#include <limits.h> // INT_MAX

//#include <complex.h>
//#include <fftw3.h>

using namespace Tiled;
using namespace Tiled::Internal;

inline bool operator==(const QPair<Tile*,Tile*> &q1, const QPair<Tile*,Tile*> &q2)
{
    return q1.first == q2.first && q1.second == q2.second;
}

inline uint qHash(const QPair<Cell,Cell> &key)
{
    return qHash(key.first) + qHash(key.second);
}



SmartTilingManager *SmartTilingManager::mInstance = 0;

SmartTilingManager::SmartTilingManager(QObject *parent)
    : QObject(parent)
    , mMapDocument(0)
    , mEnabled(false)
{
}

SmartTilingManager::~SmartTilingManager()
{

}

SmartTilingManager *SmartTilingManager::instance()
{
    if (!mInstance)
        mInstance = new SmartTilingManager();
    return mInstance;
}

void SmartTilingManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

unsigned int SmartTilingManager::getTileRelationH(Cell left, Cell right)
{
    if (mHProb.contains(QPair<Cell, Cell>(left, right)))
        return mHProb[QPair<Cell, Cell>(left, right)];
    return INT_MAX/16;
}

unsigned int SmartTilingManager::getTileRelationV(Cell top, Cell bottom)
{
    if (mVProb.contains(QPair<Cell, Cell>(top, bottom)))
        return mVProb[QPair<Cell, Cell>(top, bottom)];
    return INT_MAX/16;
}

unsigned int SmartTilingManager::getTileProb(Cell t)
{
    if (mTileProb.contains(t))
        return mTileProb[t];
    return INT_MAX/16;
}

void SmartTilingManager::setTileRelationH(Cell left, Cell right, quint64 value)
{
    mHProb[QPair<Cell, Cell>(left, right)] = value;
}

void SmartTilingManager::setTileRelationV(Cell top, Cell bottom, quint64 value)
{
    mVProb[QPair<Cell, Cell>(top, bottom)] = value;
}

void SmartTilingManager::setTileProb(Cell t, quint64 value)
{
    mTileProb[t] = value;
}

void SmartTilingManager::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);
    mMapDocument = mapDocument;
    if (mMapDocument)
        connect(mMapDocument, SIGNAL(regionEdited(QRegion,Layer*)),
                this, SLOT(smarttiling(QRegion,Layer*)));
}

QRegion neighbouringRegion(QRegion r, bool withEdges = false)
{
    QRegion ret;
    foreach (QRect rect, r.rects()) {
        if (withEdges)
            ret |= rect.adjusted(-1,-1,1,1);
        else {
            // need to unroll, because | applied to 2 rects will return bounding.
            ret |= rect.adjusted(-1,0,0,0);
            ret |= rect.adjusted(0,-1,0,0);
            ret |= rect.adjusted(0,0,1,0);
            ret |= rect.adjusted(0,0,0,1);
        }
    }
    return ret - r;
}

QList<QPoint> pointList(QRect r)
{
    QList<QPoint> ret;
    for (int x = r.left(); x <= r.right(); x++)
        for (int y = r.top(); y <= r.bottom(); y++)
            ret.append(QPoint(x,y));
    return ret;
}
#include <QDebug>
void SmartTilingManager::smarttiling(QRegion reg, Layer *l)
{
    if (!mEnabled)
        return;

    QRegion doneRegion(reg); // this region is done and won't be touched any further
    QRegion processingRegion(neighbouringRegion(doneRegion)); // currently processed
    QRegion ignoreRegion(neighbouringRegion(reg, true));
    ignoreRegion-=processingRegion;
    ignoreRegion -=doneRegion;
    QRegion newRegion(1,1,1,1); // after the current region, we should take a look here

    int i = 2;
    while (!processingRegion.isEmpty() && i--) {
        newRegion = QRegion();

        qDebug()<<"Regions:";
        qDebug()<<"processingRegion"<<processingRegion;
        qDebug()<<"doneRegion"<<doneRegion;
        qDebug()<<"ignoreRegion"<<ignoreRegion;


        foreach (QRect rect, processingRegion.rects())
            foreach (QPoint p, pointList(rect))
                calculateTile(p.x(), p.y(), static_cast<TileLayer*>(l), &newRegion, ignoreRegion, doneRegion);

//        foreach (QRect rect, processingRegion.rects())
//            foreach (QPoint p, pointList(rect))
//                calculateTile(p.x(), p.y(), static_cast<TileLayer*>(l), &newRegion, QRegion(), doneRegion);

//        qDebug()<<"newRegion"<<newRegion;

        doneRegion |= newRegion;
        processingRegion |= neighbouringRegion(newRegion);
        ignoreRegion = neighbouringRegion(newRegion, true) - processingRegion - doneRegion;
        processingRegion -= doneRegion;
        qDebug()<<"--------------------------------------------";
    }
    mMapDocument->emitRegionChanged(doneRegion);
}

bool SmartTilingManager::calculateTile(int x, int y, TileLayer *l,
                                       QRegion *workingRegion,
                                       QRegion ignoreRegion,
                                       QRegion weightRegion)
{
    if (!(l->contains(x, y)))
        return false;
//    qDebug()<<"calculate Tile"<<x << y << l << *workingRegion<< ignoreRegion<< weightRegion;
    qint64 oldSum = 0;

    int weight=3;

    QPoint there;
    QPoint here = QPoint(x,y);
    Cell hereTile = l->cellAt(here);

    there=QPoint(x - 1, y);
    if (l->contains(there) && !ignoreRegion.contains(there))
        oldSum += getTileRelationH(l->cellAt(there), hereTile)
                * getTileProb(l->cellAt(there))
                * (weightRegion.contains(there) ? weight : 1)
                * (ignoreRegion.contains(there) ? 1 : weight);

    there = QPoint(x + 1, y);
    if (l->contains(there) && !ignoreRegion.contains(there))
        oldSum += getTileRelationH(hereTile, l->cellAt(there))
                * getTileProb(l->cellAt(there))
                * (weightRegion.contains(there) ? weight : 1)
                * (ignoreRegion.contains(there) ? 1 : weight);

    there = QPoint(x, y - 1);
    if (l->contains(there) && !ignoreRegion.contains(there))
        oldSum += getTileRelationV(l->cellAt(there), hereTile)
                * getTileProb(l->cellAt(there))
                * (weightRegion.contains(there) ? weight : 1)
                * (ignoreRegion.contains(there) ? 1 : weight);

    there = QPoint(x, y + 1);
    if (l->contains(there) && !ignoreRegion.contains(there))
        oldSum += getTileRelationV(hereTile, l->cellAt(there))
                * getTileProb(l->cellAt(there))
                * (weightRegion.contains(there) ? weight : 1)
                * (ignoreRegion.contains(there) ? 1 : weight);

    qint64 newSum = oldSum;
    Cell newTile(0);
    foreach (Cell t, mTileProb.keys()) {
        Q_ASSERT(t.tile);
        qint64 sum = 0;

        there = QPoint(x - 1, y);
        if (l->contains(there) && !ignoreRegion.contains(there))
            sum += getTileRelationH(l->cellAt(there), t)
                    * getTileProb(t)
                    * (weightRegion.contains(there) ? weight : 1)
                    * (ignoreRegion.contains(there) ? 1 : weight);

        there = QPoint(x + 1, y);
        if (l->contains(there) && !ignoreRegion.contains(there))
            sum += getTileRelationH(t, l->cellAt(there))
                    * getTileProb(t)
                    * (weightRegion.contains(there) ? weight : 1)
                    * (ignoreRegion.contains(there) ? 1 : weight);

        there = QPoint(x, y - 1);
        if (l->contains(there) && !ignoreRegion.contains(there))
            sum += getTileRelationV(l->cellAt(there), t)
                    * getTileProb(t)
                    * (weightRegion.contains(there) ? weight : 1)
                    * (ignoreRegion.contains(there) ? 1 : weight);

        there = QPoint(x, y + 1);
        if (l->contains(there) && !ignoreRegion.contains(there))
            sum += getTileRelationV(t, l->cellAt(there))
                    * getTileProb(t)
                    * (weightRegion.contains(there) ? weight : 1)
                    * (ignoreRegion.contains(there) ? 1 : weight);

        if (sum < newSum) {
            newSum = sum;
            newTile = t;
        }
    }
    QRegion ret;
    if (getTileProb(l->cellAt(x,y)) + newSum < oldSum) {
        ret = QRect(x,y,1,1);
        l->setCell(x, y, newTile);
        *workingRegion |= ret;
        return true;
    }
    return false;
}

/*http://www-graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious*/
unsigned int log2(quint64 value)
{
    unsigned int ret = 0;

    while (value >>= 1) // unroll for more speed...
    {
      ret++;
    }
    return ret;
}

void SmartTilingManager::pixelLists(Cell first, Cell second, bool vertical,
                                    int distFirst, int distSecond,
                                    QList<QRgb> *destFirst,
                                    QList<QRgb> *destSecond)
{
    QImage firstimage = first.tile->image().toImage();
    QImage secondimage = second.tile->image().toImage();

    switch(mMapDocument->map()->orientation()) {
    case Map::Unknown:
        break;
    case Map::Orthogonal:
        int len;
        if (vertical) {
            Q_ASSERT(first.tile->width() == second.tile->width());
            len = first.tile->width();
        } else {
            Q_ASSERT(first.tile->height() == second.tile->height());
            len = first.tile->height();
        }
        distFirst=0;
        distSecond=0;
        for (int i = 0 ; i < len; i++) {
            if (vertical) {
                destFirst->append(firstimage.pixel(i, first.tile->height() - 1 - distFirst));
                destSecond->append(secondimage.pixel(i, distSecond));
            } else {
                destFirst->append(firstimage.pixel(first.tile->width() - 1 - distFirst, i));
                destSecond->append(secondimage.pixel(distSecond, i));
            }
        }
        break;
    case Map::Isometric:
        break;
    case Map::Hexagonal:
        break;
    }
}

int SmartTilingManager::numberOfRelationLines(Cell first, Cell second, bool vertical)
{

    switch(mMapDocument->map()->orientation()) {
    case Map::Unknown:
        return 1;
    case Map::Orthogonal:
        if (vertical)
            return qMin(first.tile->width(),second.tile->width())/4;
        else
            return qMin(first.tile->height(),second.tile->height())/4;
    case Map::Isometric:
        return 1;
    case Map::Hexagonal:
        return 1;
    }
}

void SmartTilingManager::calculateRelation(Cell first, Cell second, bool vertical)
{
    quint64 cor[4];
    memset(cor, 0, 4 * sizeof(quint64));
    int N = numberOfRelationLines(first, second, vertical);
    N=4;
    qDebug()<<"calculating Relation";
    for (int k = 0; k < N; k++)
    for (int j = 0; j < N; j++) {
        QList<QRgb> pixFirst;
        QList<QRgb> pixSecond;
        pixelLists(first, second, vertical, k, j, &pixFirst, &pixSecond);
        for (int i = 0; i < pixFirst.size(); i++) {
            quint64 w = (1<<((N-k)+(N-j)));
            QRgb firstpix = pixFirst.at(i);
            QRgb secondpixel = pixSecond.at(i);
            cor[0] += log2(w * (qRed(firstpix) - qRed(secondpixel))
                             * (qRed(firstpix) - qRed(secondpixel)));
            cor[1] += log2(w * (qGreen(firstpix) - qGreen(secondpixel))
                             * (qGreen(firstpix) - qGreen(secondpixel)));
            cor[2] += log2(w * (qBlue(firstpix) - qBlue(secondpixel))
                             * (qBlue(firstpix) - qBlue(secondpixel)));
            cor[3] += log2(w * (qAlpha(firstpix) - qAlpha(secondpixel))
                             * (qAlpha(firstpix) - qAlpha(secondpixel)));
//            qDebug()<<log2(w * (qRed(firstpix) - qRed(secondpixel))
//                             * (qRed(firstpix) - qRed(secondpixel)))<<cor[0];
//            qDebug()<<log2(w * (qGreen(firstpix) - qGreen(secondpixel))
//                             * (qGreen(firstpix) - qGreen(secondpixel)))<<cor[1];
//            qDebug()<<log2(w * (qBlue(firstpix) - qBlue(secondpixel))
//                             * (qBlue(firstpix) - qBlue(secondpixel)))<<cor[2];
//            qDebug()<<log2(w * (qAlpha(firstpix) - qAlpha(secondpixel))
//                             * (qAlpha(firstpix) - qAlpha(secondpixel)))<<cor[3];
        }
    }

    quint64 ret = 0;
    for (int i = 0; i < 4; i++)
        //ret += log2((cor[i]) * (cor[i]));
        ret+=abs((cor[i])*(cor[i]));

    if (vertical)
        setTileRelationV(first, second, ret);
    else
        setTileRelationH(first, second, ret);
}

//void SmartTilingManager::addTile(Cell t)
//{
//    Tileset *ts = t.tile->tileset();
//    for (int j = 0; j < ts->tileCount(); j++) {
//        calculateRelation(t, ts->cellAt(j), false);
//        calculateRelation(ts->cellAt(j), t, false);
//        calculateRelation(t, ts->cellAt(j), true);
//        calculateRelation(ts->cellAt(j), t, true);
//    }
//}

//void SmartTilingManager::addTileset(Tileset *ts)
//{
//    for (int i = 0; i < ts->tileCount(); i++)
//        addTile(ts->cellAt(i));
//    for (int i = 0; i < ts->tileCount(); i++)
//        mTileProb[ts->cellAt(i)]=1;
//}

void SmartTilingManager::addSmartTiles(TileLayer *stamp)
{
    foreach (Cell t1, stamp->usedCells()) {
        foreach (Cell t2, stamp->usedCells()) {
            // need only 2 directions, since the other directions come in when
            // t1 and t2 are swapped.
            calculateRelation(t1, t2, false);
            calculateRelation(t1, t2, true);
        }
        mTileProb[t1]=1;
    }
}
