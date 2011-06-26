/*
 * layerdock.cpp
 * Copyright 2011-2012, Stefan Beller <stefanbeller@googlemail.com>
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

#include "diffdock.h"

#include "changetileselection.h"
#include "documentmanager.h"
#include "geometry.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapreaderinterface.h"
#include "mapview.h"
#include "objectgrouppropertiesdialog.h"
#include "objectgroup.h"
#include "pluginmanager.h"
#include "propertiesdialog.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tmxmapreader.h"
#include "utils.h"

#include <QApplication>
#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QSettings>
#include <QSlider>
#include <QTemporaryFile>
#include <QToolBar>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;

DiffDock::DiffDock(QWidget *parent): QDockWidget(parent)
  ,mCurrentDiff(0)
  ,mOpacityLabel(new QLabel)
  ,mOpacitySlider(new QSlider(Qt::Horizontal))
  ,mMapDocument(0)
  ,mParent(parent)
{
    setObjectName(QLatin1String("diffDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);

    QVBoxLayout *diffAgainstLayout = new QVBoxLayout;
    QHBoxLayout *diffAgainstSubLayout = new QHBoxLayout;
    mDiffLine = new QLineEdit;
    QPushButton *fileNameChooser = new QPushButton;
    QIcon openIcon;
    openIcon.addFile(QLatin1String(":images/24x24/document-open.png"));

    QSettings settings;
    QString diffLineText = QLatin1String("git --no-pager show HEAD~1:%mapfile");
    QLatin1String key("Difftool/DiffLineText");
    if (settings.contains(key))
        diffLineText = settings.value(key).toString();
    mDiffLine->setText(diffLineText);

    fileNameChooser->setIcon(openIcon);
    diffAgainstSubLayout->addWidget(mDiffLine);
    diffAgainstSubLayout->addWidget(fileNameChooser);
    QPushButton *newDiffButton = new QPushButton;
    newDiffButton->setText(tr("Generate new Diff"));

    diffAgainstLayout->addLayout(diffAgainstSubLayout);
    diffAgainstLayout->addWidget(newDiffButton);

    QHBoxLayout *opacityLayout = new QHBoxLayout;
    mOpacitySlider->setRange(0, 100);
    mOpacitySlider->setEnabled(false);
    opacityLayout->addWidget(mOpacityLabel);
    opacityLayout->addWidget(mOpacitySlider);
    mOpacityLabel->setBuddy(mOpacitySlider);

    QHBoxLayout *cycleButtons = new QHBoxLayout;
    QPushButton *previousButton = new QPushButton(tr("previous Difference"));
    QPushButton *nextButton = new QPushButton(tr("next Difference"));
    cycleButtons->addWidget(previousButton);
    cycleButtons->addWidget(nextButton);

    QHBoxLayout *showButtons = new QHBoxLayout;
    QPushButton *showPlus = new QPushButton(tr("Show ++ and base"));
    QPushButton *showMinus = new QPushButton(tr("Show -- and base"));
    showButtons->addWidget(showPlus);
    showButtons->addWidget(showMinus);

    QHBoxLayout *mergeButtons = new QHBoxLayout;
    QPushButton *mergePlus = new QPushButton(tr("Merge ++ to base"));
    QPushButton *mergeMinus = new QPushButton(tr("merge -- to base"));
    mergeButtons->addWidget(mergePlus);
    mergeButtons->addWidget(mergeMinus);

    layout->addLayout(diffAgainstLayout);
    layout->addLayout(opacityLayout);
    layout->addLayout(cycleButtons);
    layout->addLayout(showButtons);
    layout->addLayout(mergeButtons);
    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding,
                                          QSizePolicy::Expanding);
    layout->addSpacerItem(spacer);

    setWidget(widget);
    retranslateUi();

    connect(fileNameChooser, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(newDiffButton, SIGNAL(clicked()), this, SLOT(newDiff()));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(previousDifference()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextDifference()));
    connect(showPlus, SIGNAL(clicked()), this, SLOT(showPlus()));
    connect(showMinus, SIGNAL(clicked()), this, SLOT(showMinus()));
    connect(mergePlus, SIGNAL(clicked()), this, SLOT(mergePlusIntoBase()));
    connect(mergeMinus, SIGNAL(clicked()), this, SLOT(mergeMinusIntoBase()));

    connect(mOpacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(setLayerOpacity(int)));
    updateOpacitySlider();
}

void DiffDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(currentLayerIndexChanged(int)),
                this, SLOT(updateOpacitySlider()));
    }
    updateOpacitySlider();
}

void DiffDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void DiffDock::updateOpacitySlider()
{
    const bool enabled = mMapDocument &&
                         mMapDocument->currentLayerIndex() != -1;

    mOpacitySlider->setEnabled(enabled);
    mOpacityLabel->setEnabled(enabled);

    if (enabled) {
        qreal opacity = mMapDocument->currentLayer()->opacity();
        mOpacitySlider->setValue((int) (opacity * 100));
    } else {
        mOpacitySlider->setValue(100);
    }
}

void DiffDock::setLayerOpacity(int opacity)
{
    if (!mMapDocument)
        return;

    const int layerIndex = mMapDocument->currentLayerIndex();
    if (layerIndex == -1)
        return;

    foreach (Layer *layer, mMapDocument->map()->layers()) {
        if (layer->name().startsWith(QLatin1String("++")) ||
                layer->name().startsWith(QLatin1String("--")))
            continue;

        if ((int) (layer->opacity() * 100) != opacity) {
            LayerModel *layerModel = mMapDocument->layerModel();
            const int row = layerModel->layerIndexToRow(layerIndex);
            layerModel->setData(layerModel->index(row),
                                qreal(opacity) / 100,
                                LayerModel::OpacityRole);
        }
    }
}

void DiffDock::retranslateUi()
{
    setWindowTitle(tr("DiffTool"));
    mOpacityLabel->setText(tr("Common Layers opacity:"));
}


void DiffDock::chooseFile()
{
    QSettings mSettings;
    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    mSelectedFilter = tr("Tiled map files (*.tmx)");
    filter += mSelectedFilter;

    mSelectedFilter = mSettings.value(QLatin1String("lastUsedOpenFilter"),
                                     mSelectedFilter).toString();

    const PluginManager *pm = PluginManager::instance();
    QList<MapReaderInterface*> readers = pm->interfaces<MapReaderInterface>();
    foreach (MapReaderInterface *reader, readers) {
        foreach (const QString &str, reader->nameFilters()) {
            if (!str.isEmpty()) {
                filter += QLatin1String(";;");
                filter += str;
            }
        }
    }

    QString fileDialogStartLocation;
    if (mMapDocument)
        fileDialogStartLocation =
                QFileInfo(mMapDocument->fileName()).canonicalPath();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Map"),
                                                    fileDialogStartLocation,
                                                    filter, &mSelectedFilter);
    if (fileName.isEmpty())
        return;

    mDiffLine->setText(fileName);
}

void DiffDock::newDiff()
{
    if (!mMapDocument)
        return;

    QString text = mDiffLine->text();
    QSettings settings;
    settings.setValue(QLatin1String("Difftool/DiffLineText"), mDiffLine->text());
    if (QFileInfo(text).exists())
        newDiffByFileName(text);
    else
        newDiffByProgram(text);
}

void DiffDock::newDiffByProgram(QString command)
{
    QProcess builder;

    if (QFileInfo(mMapDocument->fileName()).exists())
        builder.setWorkingDirectory(
                    QFileInfo(mMapDocument->fileName()).canonicalPath());

    QDir dir(builder.workingDirectory());

    command.replace(QLatin1String("%mapfile"),
                    QString(QLatin1String("./%1"))
                    .arg(dir.relativeFilePath(mMapDocument->fileName())));

    builder.setProcessChannelMode(QProcess::SeparateChannels);

    QTemporaryFile *file = new QTemporaryFile(
                builder.workingDirectory()
                + QLatin1String("/qt-temp-XXXXXX"));
    file->open();
    builder.setStandardOutputFile(file->fileName());
    builder.setStandardErrorFile(file->fileName());

    builder.start(command);

    if (!builder.waitForFinished())
        QMessageBox::warning(mParent, tr("Process %1 failed").arg(command),
                             builder.errorString());
    else
        newDiffByFileName(file->fileName());

    delete file;
}

void DiffDock::newDiffByFileName(QString fileName)
{
    const PluginManager *pm = PluginManager::instance();
    QList<MapReaderInterface*> readers = pm->interfaces<MapReaderInterface>();
    // When a particular filter was selected, use the associated reader
    MapReaderInterface *mapReader = 0;
    foreach (MapReaderInterface *reader, readers)
        if (reader->nameFilters().contains(mSelectedFilter))
            mapReader = reader;

    TmxMapReader tmxMapReader;

    if (!mapReader && !tmxMapReader.supportsFile(fileName)) {
        // Try to find a plugin that implements support for this format
        const PluginManager *pm = PluginManager::instance();
        QList<MapReaderInterface*> readers =
                pm->interfaces<MapReaderInterface>();

        foreach (MapReaderInterface *reader, readers) {
            if (reader->supportsFile(fileName)) {
                mapReader = reader;
                break;
            }
        }
    }

    if (!mapReader)
        mapReader = &tmxMapReader;


    Map *map = mapReader->read(fileName);
    if (!map)
        return;

    foreach (Tileset *t, map->tilesets()) {
        Tileset *ts = t->findSimilarTileset(mMapDocument->map()->tilesets());
        if (ts)
            map->replaceTileset(t, ts);
    }
    diffTo(mMapDocument->map(), map);
}

void DiffDock::diffTo(Map *mapA, Map *mapB)
{
    const QString warningMessageTitle = tr("Map Diff");

    if (mapA->orientation() != mapB->orientation()) {
        QMessageBox::warning(this, warningMessageTitle,
                             tr("Maps have different orientation!"));
        return;
    }

    if (mapA->tileWidth() != mapB->tileWidth()) {
        QMessageBox::warning(this, warningMessageTitle,
                             tr("Maps have different tile width!"));
        return;
    }

    if (mapA->tileHeight() != mapB->tileHeight()) {
        QMessageBox::warning(this, warningMessageTitle,
                             tr("Maps have different tile height!"));
        return;
    }

    Map *diffMap = new Map(mapA->orientation(),
                           std::max(mapA->width(), mapB->width()),
                           std::max(mapA->height(), mapB->height()),
                           mapA->tileWidth(), mapA->tileHeight());

    foreach (Layer *layerA, mapA->layers()) {
        Layer* matchedLayerB = 0;
        foreach (Layer *layerB, mapB->layers())
            if (layerB->name() == layerA->name())
                matchedLayerB = layerB;

        if (matchedLayerB)
            if (layerA->asTileLayer() && matchedLayerB->asTileLayer()) {
                TileLayer *tileLayerA = layerA->asTileLayer();
                TileLayer *tileLayerB = matchedLayerB->asTileLayer();

                QRegion regionDiffA = tileLayerA->computeDiffRegion(tileLayerB);
                QRegion regionDiffB = tileLayerB->computeDiffRegion(tileLayerA);
                QRegion regionDiff = regionDiffA | regionDiffB;
                QRegion regionNotDiff = tileLayerA->region() - regionDiff;
                TileLayer *base = tileLayerA->copy(regionNotDiff);
                TileLayer *diffA = tileLayerA->copy(regionDiffA);
                TileLayer *diffB = tileLayerB->copy(regionDiffB);

                const QString name = layerA->name();
                base->setName(name);
                base->setOpacity(0.15);
                diffA->setName(QLatin1String("++") + name);
                diffB->setName(QLatin1String("--") + name);

                diffA->resize(tileLayerA->bounds().size(),
                              regionDiffA.boundingRect().topLeft());
                diffB->resize(tileLayerB->bounds().size(),
                              regionDiffB.boundingRect().topLeft());

                const int index = diffMap->layerCount();
                diffMap->insertLayer(index , base);
                if (!diffA->isEmpty())
                    diffMap->insertLayer(index + 1, diffA);
                if (!diffB->isEmpty())
                    diffMap->insertLayer(index + 1, diffB);
            }
    }
    MapDocument *mapDocument = new MapDocument(diffMap, QString());
    emit addMapDocument(mapDocument);
}

QList<QRegion> DiffDock::getDifferenceRegions()
{
    Map *map = mMapDocument->map();
    TileLayer *baseLayer = new TileLayer(QString(), 0, 0,
                                     map->width(), map->height());
    foreach (Layer *layer, map->layers()) {
        TileLayer *tl = layer->asTileLayer();
        if (!tl)
            continue;
        if (tl->name().startsWith(QLatin1String("++")) ||
                tl->name().startsWith(QLatin1String("--")))
            baseLayer->merge(QPoint(), tl);
    }
    QList<QRegion> ret = coherentRegions(baseLayer->region());
    delete baseLayer;
    return ret;
}

QPoint DiffDock::getMidPoint(QRegion region)
{
    if (region.isEmpty())
        return QPoint();

    int resx = 0;
    int resy = 0;
    int allweight = 0;
    foreach (QRect rect, region.rects()) {
        int weight = rect.width() * rect.height();
        allweight += weight;
        int x = (rect.right() - rect.left())/2 + rect.left();
        int y = (rect.top() - rect.bottom())/2 + rect.bottom();
        resx += weight * x;
        resy += weight * y;
    }
    return QPoint((int)(resx / allweight), (int)(resy / allweight));
}

void DiffDock::markDifference()
{
    QList<QRegion> regions = getDifferenceRegions();

    if (regions.size() == 0)
        return;
    else if (mCurrentDiff < 0)
        mCurrentDiff = regions.size() - 1;
    else if (mCurrentDiff >= regions.size())
        mCurrentDiff = 0;

    QRegion diff = regions.at(mCurrentDiff);

    Q_ASSERT (!diff.isEmpty());

    QPoint mid = getMidPoint(diff);
    DocumentManager::instance()->centerViewOn(mid.x(), mid.y());
    QUndoStack *stack = mMapDocument->undoStack();
    stack->push(new ChangeTileSelection(mMapDocument, diff));
}

void DiffDock::previousDifference()
{
    mCurrentDiff--;
    markDifference();
}

void DiffDock::nextDifference()
{
    mCurrentDiff++;
    markDifference();
}

void DiffDock::mergePlusIntoBase()
{
    Map *map = mMapDocument->map();
    QRegion region = mMapDocument->tileSelection();
    if (region.isEmpty())
        return;
    if (!map)
        return;
    foreach (Layer *layer, map->layers()) {
        QString name = layer->name();
        if (name.startsWith(QLatin1String("++"))) {
            name.remove(0, 2);
            int index = map->indexOfLayer(name);
            TileLayer *tlbase = map->layerAt(index)->asTileLayer();
            TileLayer *tlplus = layer->asTileLayer();
            if (!tlbase || !tlplus)
                continue;

            TileLayer *tl = tlplus->copy(region);
            tlbase->merge(region.boundingRect().topLeft(), tl);
            tlplus->erase(region);

            index = map->indexOfLayer(QLatin1String("--") + name);
            if (index == -1)
                continue;
            TileLayer *tlminus = map->layerAt(index)->asTileLayer();
            if (!tlminus)
                continue;
            tlminus->erase(region);
        }
    }
    mMapDocument->emitRegionChanged(region);
}

void DiffDock::mergeMinusIntoBase()
{
    Map *map = mMapDocument->map();
    QRegion region = mMapDocument->tileSelection();
    if (region.isEmpty())
        return;
    if (!map)
        return;
    foreach (Layer *layer, map->layers()) {
        QString name = layer->name();
        if (name.startsWith(QLatin1String("--"))) {
            name.remove(0, 2);
            int index = map->indexOfLayer(name);
            TileLayer *tlbase = map->layerAt(index)->asTileLayer();
            TileLayer *tlminus = layer->asTileLayer();
            if (!tlbase || !tlminus)
                continue;

            TileLayer *tl = tlminus->copy(region);
            tlbase->merge(region.boundingRect().topLeft(), tl);
            tlminus->erase(region);

            index = map->indexOfLayer(QLatin1String("++") + name);
            if (index == -1)
                continue;
            TileLayer *tlplus = map->layerAt(index)->asTileLayer();
            if (!tlplus)
                continue;
            tlplus->erase(region);
        }
    }
    mMapDocument->emitRegionChanged(region);
}

void DiffDock::showPlus()
{
    LayerModel *ml = mMapDocument->layerModel();
    for (int i = 0; i < ml->rowCount(); ++i) {
        QModelIndex index = ml->index(i);
        if (ml->data(index, Qt::EditRole)
                .toString().startsWith(QLatin1String("++")))
            ml->setData(index, Qt::Checked, Qt::CheckStateRole);
        else if (ml->data(index, Qt::EditRole)
                 .toString().startsWith(QLatin1String("--")))
            ml->setData(index, 0, Qt::CheckStateRole);
        else
            ml->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}

void DiffDock::showMinus()
{
    LayerModel *ml = mMapDocument->layerModel();
    for (int i = 0; i < ml->rowCount(); ++i) {
        QModelIndex index = ml->index(i);
        if (ml->data(index, Qt::EditRole)
                .toString().startsWith(QLatin1String("++")))
            ml->setData(index, 0, Qt::CheckStateRole);
        else if (ml->data(index, Qt::EditRole)
                 .toString().startsWith(QLatin1String("--")))
            ml->setData(index, Qt::Checked, Qt::CheckStateRole);
        else
            ml->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}

