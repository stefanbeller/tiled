/*
 * documentmanager.cpp
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "documentmanager.h"

#include "abstracttool.h"
#include "filesystemwatcher.h"
#include "maprenderer.h"
#include "toolmanager.h"

#include <QPushButton>
#include <QFileInfo>
#include <QObjectList>
#include <QSignalMapper>
#include <QTabWidget>
#include <QUndoGroup>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

DocumentManager *DocumentManager::mInstance = 0;

DocumentManager *DocumentManager::instance()
{
    if (!mInstance)
        mInstance = new DocumentManager;
    return mInstance;
}

void DocumentManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
    , mTabWidget(new QTabWidget)
    , mUndoGroup(new QUndoGroup(this))
    , mSelectedTool(0)
    , mSceneWithTool(0)
    , mFileSystemWatcher(new FileSystemWatcher)
    , mSignalMapper(new QSignalMapper)
{
    mTabWidget->setDocumentMode(true);
    mTabWidget->setTabsClosable(true);

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(currentIndexChanged()));
    connect(mTabWidget, SIGNAL(tabCloseRequested(int)),
            SIGNAL(documentCloseRequested(int)));

    ToolManager *toolManager = ToolManager::instance();
    setSelectedTool(toolManager->selectedTool());
    connect(toolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            SLOT(setSelectedTool(AbstractTool*)));

    connect(mFileSystemWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));
    connect(mSignalMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(removeReloadWarning(QObject*)));
}

DocumentManager::~DocumentManager()
{
    // All documents should be closed gracefully beforehand
    Q_ASSERT(mDocuments.isEmpty());
    delete mFileSystemWatcher;
    delete mTabWidget;
}

QWidget *DocumentManager::widget() const
{
    return mTabWidget;
}

MapDocument *DocumentManager::currentDocument() const
{
    const int index = mTabWidget->currentIndex();
    if (index == -1)
        return 0;

    return mDocuments.at(index);
}

MapView *DocumentManager::currentMapView() const
{
    const QWidget *w = mTabWidget->currentWidget();
    if (!w)
        return 0;
    const QObjectList children = w->children();
    return static_cast<MapView*>(children.at(1));
}

MapScene *DocumentManager::currentMapScene() const
{
    if (MapView *mapView = currentMapView())
        return mapView->mapScene();

    return 0;
}

int DocumentManager::findDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty()) // file doesn't exist
        return -1;

    for (int i = 0; i < mDocuments.size(); ++i) {
        QFileInfo fileInfo(mDocuments.at(i)->fileName());
        if (fileInfo.canonicalFilePath() == canonicalFilePath)
            return i;
    }

    return -1;
}

void DocumentManager::switchToDocument(int index)
{
    mTabWidget->setCurrentIndex(index);
}

void DocumentManager::switchToLeftDocument()
{
    const int tabCount = mTabWidget->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabWidget->currentIndex();
    switchToDocument((currentIndex > 0 ? currentIndex : tabCount) - 1);
}

void DocumentManager::switchToRightDocument()
{
    const int tabCount = mTabWidget->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabWidget->currentIndex();
    switchToDocument((currentIndex + 1) % tabCount);
}

void DocumentManager::addDocument(MapDocument *mapDocument)
{
    Q_ASSERT(mapDocument);
    Q_ASSERT(!mDocuments.contains(mapDocument));

    mDocuments.append(mapDocument);
    mUndoGroup->addStack(mapDocument->undoStack());

    QWidget *centralWidget = new QWidget(mTabWidget);
    centralWidget->setLayout(new QVBoxLayout());
    MapView *view = new MapView(centralWidget);
    MapScene *scene = new MapScene(view); // scene is owned by the view
    centralWidget->layout()->addWidget(view);

    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    const int documentIndex = mDocuments.size() - 1;

    mTabWidget->addTab(centralWidget, mapDocument->displayName());
    mTabWidget->setTabToolTip(documentIndex, mapDocument->fileName());
    connect(mapDocument, SIGNAL(fileNameChanged()), SLOT(updateDocumentTab()));
    connect(mapDocument, SIGNAL(modifiedChanged()), SLOT(updateDocumentTab()));

    // keep us in sync with disc
    mFileSystemWatcher->addPath(mapDocument->fileName());
    mSignalMapper->setMapping(mapDocument, mapDocument);
    connect(mapDocument, SIGNAL(fileSaved()),
            mSignalMapper, SLOT(map()));

    switchToDocument(documentIndex);
    centerViewOn(0, 0);
}

void DocumentManager::closeCurrentDocument()
{
    const int index = mTabWidget->currentIndex();
    if (index == -1)
        return;

    MapDocument *mapDocument = mDocuments.takeAt(index);
    MapView *mapView = currentMapView();

    mTabWidget->removeTab(index);
    delete mapView;
    delete mapDocument;
}

void DocumentManager::closeAllDocuments()
{
    while (!mDocuments.isEmpty())
        closeCurrentDocument();
}

void DocumentManager::currentIndexChanged()
{
    if (mSceneWithTool) {
        mSceneWithTool->disableSelectedTool();
        mSceneWithTool = 0;
    }

    MapDocument *mapDocument = currentDocument();

    if (mapDocument)
        mUndoGroup->setActiveStack(mapDocument->undoStack());

    emit currentDocumentChanged(mapDocument);

    if (MapScene *mapScene = currentMapScene()) {
        mapScene->setSelectedTool(mSelectedTool);
        mapScene->enableSelectedTool();
        mSceneWithTool = mapScene;
    }
}

void DocumentManager::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    mSelectedTool = tool;

    if (mSceneWithTool) {
        mSceneWithTool->disableSelectedTool();

        if (mSelectedTool) {
            mSceneWithTool->setSelectedTool(mSelectedTool);
            mSceneWithTool->enableSelectedTool();
        }
    }
}

void DocumentManager::updateDocumentTab()
{
    MapDocument *mapDocument = static_cast<MapDocument*>(sender());
    const int index = mDocuments.indexOf(mapDocument);

    QString tabText = mapDocument->displayName();
    if (mapDocument->isModified())
        tabText.prepend(QLatin1Char('*'));

    mTabWidget->setTabText(index, tabText);
    mTabWidget->setTabToolTip(index, mapDocument->fileName());
}

void DocumentManager::centerViewOn(int x, int y)
{
    MapView *view = currentMapView();
    if (!view)
        return;

    view->centerOn(currentDocument()->renderer()->tileToPixelCoords(x, y));
}

void DocumentManager::addReloadWarning(MapDocument *whichDocument)
{
    const int index = mDocuments.indexOf(whichDocument);
    QVBoxLayout *layout = static_cast<QVBoxLayout*>
            (mTabWidget->widget(index)->layout());

    if (dynamic_cast<QPushButton*>(layout->itemAt(0)->widget()))
        return;

    QPushButton *warningMessage = new QPushButton();
    warningMessage->setText(tr("File changed on disc! Click here to reload."));
    layout->insertWidget(0, warningMessage);
}

void DocumentManager::removeReloadWarning(QObject *whichObject)
{
    MapDocument *whichDocument =static_cast<MapDocument*>(whichObject);
    const int index = mDocuments.indexOf(whichDocument);
    QVBoxLayout *layout = static_cast<QVBoxLayout*>
            (mTabWidget->widget(index)->layout());

    QPushButton *warningMessage = dynamic_cast<QPushButton*>
            (layout->itemAt(0)->widget());

    if (warningMessage)
        layout->removeWidget(warningMessage);
}

void DocumentManager::fileChanged(const QString &fileName)
{
    foreach (MapDocument *mapDocument, mDocuments) {
        if (fileName == mapDocument->fileName()) {
            addReloadWarning(mapDocument);
        }
    }
}
