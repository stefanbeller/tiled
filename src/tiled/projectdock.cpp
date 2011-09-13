/*
 * projectdock.cpp
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

#include "projectdock.h"

#include "projectactionhandler.h"
#include "projecttreemodel.h"
#include "projecttreeitem.h"
#include "projectview.h"
#include "project.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QEvent>
#include <QFileSystemModel>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>


#include <QDebug>

#include "utils.h"

using namespace Tiled;
using namespace Tiled::Internal;

ProjectDock::ProjectDock(ProjectTreeModel *m, QWidget *parent)
        : QDockWidget(parent)
        , mTreeView(0)
        , mProjectTreeModel(m)
{
    setObjectName(QLatin1String("projectDock"));

    ProjectActionHandler *handler = ProjectActionHandler::instance();

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    mTreeView = new ProjectView(widget);
    mTreeView->setModel(m);
    mTreeView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QMenu *newMenu = new QMenu(this);
    newMenu->addAction(handler->actionAddMap());
    newMenu->addAction(handler->actionNewMap());
    newMenu->addAction(handler->actionNewProject());

    const QIcon newIcon(QLatin1String(":/images/16x16/document-new.png"));
    QToolButton *newButton = new QToolButton;
    newButton->setPopupMode(QToolButton::InstantPopup);
    newButton->setMenu(newMenu);
    newButton->setIcon(newIcon);
    Utils::setThemeIcon(newButton, "document-new");

    QToolBar *buttonContainer = new QToolBar;
    buttonContainer->setFloatable(false);
    buttonContainer->setMovable(false);
    buttonContainer->setIconSize(QSize(16, 16));

    buttonContainer->addWidget(newButton);
    layout->addWidget(buttonContainer);

    connect (mTreeView, SIGNAL(doubleClicked(const QModelIndex&)),
                this, SLOT(doubleClick(const QModelIndex&)));
    layout->addWidget(mTreeView);
    setWidget(widget);

    connect (mTreeView, SIGNAL(doubleClicked(QModelIndex)),
             SLOT(doubleClick(QModelIndex)));
    retranslateUi();
}

ProjectDock::~ProjectDock()
{
    delete mTreeView;
}

void ProjectDock::doubleClick(const QModelIndex &index)
{
    ProjectTreeItem *pti = mProjectTreeModel->getItem(index);
    qDebug()<<"doubleClick"<< index <<pti->isProject()<< pti->isMap()<<pti->isMapFolder()<<pti->isTileset()<<pti->isTilesetFolder()<<pti->isRule()<<pti->isRuleFolder()<<pti->childNumber();

    if (pti->isMap())
        emit openFile(pti->fullPath());
}

void ProjectDock::retranslateUi()
{
    setWindowTitle(tr("Projects"));
}

void ProjectDock::changeEvent(QEvent *e)
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
