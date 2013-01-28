/*
 * animationmanager.h
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>

#include <QTimer>
#include <QSet>

namespace Tiled {

class Tileset;

namespace Internal {

class MapDocument;
class MapScene;
class MapView;

class AnimationManager : public QObject
{
    Q_OBJECT

public:
    static AnimationManager *instance();
    static void deleteInstance();

    void addTileset(Tileset *tileset);
    void removeTileset(Tileset *tileset);

signals:
    /**
     * Emitted when an animation has changed.
     */
    void animationChanged();

private slots:
    void animationUpdate();
    void documentChanged(MapDocument *mapDocument);

private:
    AnimationManager();
    ~AnimationManager();

    QTimer mAnimationTimer;
    QSet<Tileset*> mTilesets;

    static AnimationManager *mInstance;
};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // ANIMATIONMANAGER_H
