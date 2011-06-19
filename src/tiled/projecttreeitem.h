/*
 * projecttreeitem.h
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

#ifndef PROJECTTREEITEM_H
#define PROJECTTREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

namespace Tiled {
namespace Internal {
class ProjectTreeItem
{

public:
    ProjectTreeItem(const QString &data, ProjectTreeItem *parent = 0);
    ~ProjectTreeItem();

    ProjectTreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QString data() const;
    QString fullPath() const;
    bool insertColumns(int position, int columns);
    ProjectTreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(const QString &value);

    bool addChild(ProjectTreeItem *pti);

    bool isMap() const;
    bool isMapFolder() const;

    bool isTileset() const;
    bool isTilesetFolder() const;

    bool isRule() const;
    bool isRuleFolder() const;

    bool isProject() const;

private:
    QList<ProjectTreeItem*> childItems;
    QString itemData;
    ProjectTreeItem *parentItem;
};

} // namespace Internal
} // namespace Tiled

#endif // PROJECTTREEITEM_H
