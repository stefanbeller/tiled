/*
 * projecttreeitem.cpp
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

#include <QStringList>

#include "projecttreeitem.h"
#include <QDebug>

using namespace Tiled;
using namespace Tiled::Internal;

ProjectTreeItem::ProjectTreeItem(const QString &data, ProjectTreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}

ProjectTreeItem::~ProjectTreeItem()
{
    qDeleteAll(childItems);
}

ProjectTreeItem *ProjectTreeItem::child(int number)
{
    return childItems.value(number);
}

int ProjectTreeItem::childCount() const
{
    return childItems.count();
}

int ProjectTreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ProjectTreeItem*>(this));
    else
        return 0;
}

int ProjectTreeItem::columnCount() const
{
    return 1;
}

QString ProjectTreeItem::data() const
{
    return itemData;
}

bool ProjectTreeItem::addChild(ProjectTreeItem *pti)
{
    childItems.append(pti);
    return true;
}

ProjectTreeItem *ProjectTreeItem::parent()
{
    return parentItem;
}

bool ProjectTreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

QString ProjectTreeItem::fullPath() const
{
    if (!isMap() && !isTileset() && !isRule())
        return QString();

    return parentItem->parentItem->data() + QLatin1String("/") + itemData;
}

bool ProjectTreeItem::setData(const QString &value)
{
    itemData = value;
    return true;
}

bool ProjectTreeItem::isMap() const
{
    return parentItem && parentItem->isMapFolder();
}

bool ProjectTreeItem::isMapFolder() const
{
    // has project as parentItem and is the first item listed
    return parentItem && parentItem->isProject() && childNumber() == 0;
}

bool ProjectTreeItem::isTileset() const
{
    return parentItem && parentItem->isTilesetFolder();
}

bool ProjectTreeItem::isTilesetFolder() const
{
    // has project as parentItem and is the second item listed
    return parentItem && parentItem->isProject() && childNumber() == 1;
}

bool ProjectTreeItem::isRule() const
{
    return parentItem && parentItem->isRuleFolder();
}

bool ProjectTreeItem::isRuleFolder() const
{
    // has project as parentItem and is the third item listed
    return parentItem && parentItem->isProject() && childNumber() == 2;
}

bool ProjectTreeItem::isProject() const
{
    // has rootItem as parentItem
    return parentItem && !parentItem->parentItem;
}
