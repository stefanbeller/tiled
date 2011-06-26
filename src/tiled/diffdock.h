/*
 * diffdock.h
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

#ifndef DIFFDOCK_H
#define DIFFDOCK_H

#include "mapdocument.h"

#include <QDockWidget>
#include <QTreeView>
#include <QToolButton>

class QLabel;
class QLineEdit;

namespace Tiled {

class Map;

namespace Internal {

class LayerView;

/**
 * The dock widget that displays the map layers.
 */
class DiffDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    DiffDock(QWidget *parent = 0);

    /**
     * Sets the map for which the layers should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

signals:
    void addMapDocument(MapDocument *mapDocument);

protected:
    void changeEvent(QEvent *e);

private slots:
    void updateOpacitySlider();
    void setLayerOpacity(int opacity);

    void chooseFile();
    void newDiff();

    void previousDifference();
    void nextDifference();

    void mergePlusIntoBase();
    void mergeMinusIntoBase();

    void showMinus();
    void showPlus();

private:
    void retranslateUi();

    void markDifference();
    void newDiffByFileName(QString fileName);
    void newDiffByProgram(QString command);
    void diffTo(Map *mapA, Map *mapB);

    QList<QRegion> getDifferenceRegions();
    QList<QRegion> mDiffs;
    int mCurrentDiff;

    QPoint getMidPoint(QRegion region);

    QString mSelectedFilter;
    QLineEdit *mDiffLine;
    QLabel *mOpacityLabel;
    QSlider *mOpacitySlider;
    MapDocument *mMapDocument;
    QWidget *mParent;
};

} // namespace Internal
} // namespace Tiled

#endif // DIFFDOCK_H
