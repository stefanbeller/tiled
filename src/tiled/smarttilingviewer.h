/*
 * smarttilingviewer.h
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
#ifndef SMARTTILINGVIEWER_H
#define SMARTTILINGVIEWER_H

#include "tile.h"

#include <QObject>
#include <QDialog>

namespace Tiled {
namespace Internal {

class SmartTilingViewer : public QDialog
{
    Q_OBJECT
public:
    SmartTilingViewer(const QString &kind,
                     Tile *object,
                     QWidget *parent = 0);
    ~SmartTilingViewer();


signals:

public slots:
    void recalculateClicked();

private:
    Tile *mTile;
    QString mKind;
};

}
}
#endif // SmartTilingVIEWER_H
