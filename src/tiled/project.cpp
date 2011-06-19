/*
 * project.cpp
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

#include "project.h"

#include <QFileInfo>

using namespace Tiled;
using namespace Tiled::Internal;


Project::Project(QString fileName)
{
    mFileName=fileName;
}

Project::~Project()
{

}

void Project::addMap(QString fileName)
{
    mMaps.append(fileName);
}

void Project::addTileset(QString fileName)
{
    mTilesets.append(fileName);
}

void Project::addRule(QString fileName)
{
    mRules.append(fileName);
}

void Project::addMaps(QStringList fileNames)
{
    foreach(QString name, fileNames)
        if (QFileInfo(mFileName + QLatin1Char('/') + name).exists() &&
                !name.isEmpty())
            mMaps.append(name);
}

void Project::addTilesets(QStringList fileNames)
{
    foreach(QString name, fileNames)
        if (QFileInfo(mFileName + QLatin1Char('/') + name).exists() &&
                !name.isEmpty())
            mTilesets.append(name);
}

void Project::addRules(QStringList fileNames)
{
    mRules.append(fileNames);
}

void Project::setRulesSetLayerName(QString setLayerName)
{
    mRulesSetLayer = setLayerName;
}

#include <QDebug>
void Project::dumpInfo()
{
    qDebug()<<this->fileName();
    qDebug()<<this->rulesSetLayerName();
    qDebug()<<"maps"<<mMaps;
    qDebug()<<"tilesets"<<mTilesets;
    qDebug()<<"rules"<<mRules;
}
