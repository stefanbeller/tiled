/*
 * projecttreemodel.cpp
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

#include "projecttreemodel.h"

#include <QtGui>
#include <QDebug>
#include <QStandardItem>
#include <QTreeWidgetItem>

#include "projectactionhandler.h"
#include "projectreader.h"
#include "projecttreeitem.h"
#include "projecttreemodel.h"

using namespace Tiled;
using namespace Tiled::Internal;

ProjectTreeModel::ProjectTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , rootItem(0)
{
    rootItem = new ProjectTreeItem(QLatin1String("Projects"));

    ProjectActionHandler *handler = ProjectActionHandler::instance();
    handler->setProjectTreeModel(this);

    ProjectReader p;
    this->addProject(p.readProject(QLatin1String("/home/sb/stefanbellers-tiled-qt/examples/exampleproject.tiled")));
    this->addProject(p.readProject(QLatin1String("/home/sb/tiled/tiled-qt/examples/exampleproject.tiled")));
}


ProjectTreeModel::~ProjectTreeModel()
{
    delete rootItem;
}

void ProjectTreeModel::addProject(Project *project)
{
    if (!project)
        return;
    ProjectTreeItem *projectItem =  new ProjectTreeItem(project->fileName(),rootItem);
    ProjectTreeItem *mapsItem =     new ProjectTreeItem(tr("maps"),projectItem);
    ProjectTreeItem *tilesetsItem = new ProjectTreeItem(tr("tilesets"),projectItem);
    ProjectTreeItem *rulesItem =    new ProjectTreeItem(tr("rules"),projectItem);

    foreach (QString map, project->maps()) {
        ProjectTreeItem *mapItem =  new ProjectTreeItem(map,mapsItem);

        mapsItem->addChild(mapItem);
    }
    foreach (QString tileset, project->tilesets()) {
        ProjectTreeItem *tilesetItem =  new ProjectTreeItem(tileset,tilesetsItem);

        tilesetsItem->addChild(tilesetItem);
    }
    foreach (QString rule, project->rules()) {
        ProjectTreeItem *ruleItem =  new ProjectTreeItem(rule,rulesItem);

        rulesItem->addChild(ruleItem);
    }
    projectItem->addChild(mapsItem);
    projectItem->addChild(tilesetsItem);
    projectItem->addChild(rulesItem);
    beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount() + 1);
    rootItem->addChild(projectItem);
    mProjects.append(project);
    endInsertRows();
    //qDebug()<<"added child to rootitem::::::::::";
//    this->rowCount();
//    this->rowCount();
//    this->columnCount();
//    this->rowCount();
//    qDebug()<<"added child to rootitem::::::::::end";
}

int ProjectTreeModel::columnCount(const QModelIndex & /* parent */) const
{
//    qDebug()<<"ProjectTreeModel::columnCount(const QModelIndex & /* parent */) const"<<rootItem->columnCount();
    return rootItem->columnCount();
}

QVariant ProjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ProjectTreeItem *item = getItem(index);

    return item->data();
}

Qt::ItemFlags ProjectTreeModel::flags(const QModelIndex &index) const
{
//    qDebug()<<"Qt::ItemFlags ProjectTreeModel::flags(const QModelIndex &index) const"<<index;

    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

ProjectTreeItem *ProjectTreeModel::getItem(const QModelIndex &index) const
{
//    qDebug()<<"ProjectTreeItem *ProjectTreeModel::getItem(const QModelIndex &index) const";
    if (index.isValid()) {
        ProjectTreeItem *item = static_cast<ProjectTreeItem*>(index.internalPointer());
        if (item) return item;
    }
//    qDebug()<<"return rootItem";
    return rootItem;
}

QVariant ProjectTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    // TODO section?
    section = section;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data();

    return QVariant();
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    ProjectTreeItem *parentItem = getItem(parent);

    ProjectTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool ProjectTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    ProjectTreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->addChild(0);
    endInsertRows();

    return success;
}

QModelIndex ProjectTreeModel::parent(const QModelIndex &index) const
{
//    qDebug()<<"QModelIndex ProjectTreeModel::parent(const QModelIndex &index) const";
    if (!index.isValid())
        return QModelIndex();

    ProjectTreeItem *childItem = getItem(index);
    ProjectTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

//    qDebug()<<parentItem;
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool ProjectTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
//    qDebug()<<"bool ProjectTreeModel::removeRows(int position, int rows, const QModelIndex &parent)!!!!!!!!!!!!!!!!!!";
    ProjectTreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int ProjectTreeModel::rowCount(const QModelIndex &parent) const
{
//    qDebug()<<"int ProjectTreeModel::rowCount(const QModelIndex &parent) const";
    ProjectTreeItem *parentItem = getItem(parent);
//    qDebug()<<"return "<<parentItem->childCount();
//    qDebug()<<"return "<<rootItem->childCount();
    return parentItem->childCount();
}

bool ProjectTreeModel::setData(const QModelIndex &index, const QString &value,
                        int role)
{
    if (role != Qt::EditRole)
        return false;

    ProjectTreeItem *item = getItem(index);
    bool result = item->setData(value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool ProjectTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QString &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

Project *ProjectTreeModel::getProjectByIndex(const QModelIndex &index)
{
    ProjectTreeItem *item = getItem(index);
    do {
        if (item->isProject())
            return mProjects.at(item->childNumber());
        item = item->parent();
    } while (item != rootItem);
    return 0;
}
