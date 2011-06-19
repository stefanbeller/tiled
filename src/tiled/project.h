/*
 * project.h
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

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QStringList>

namespace Tiled {
namespace Internal {
class Project
{
public:
    Project(QString fileName);
    ~Project();

    void addMap(QString fileName);
    void addTileset(QString fileName);
    void addRule(QString fileName);

    void addMaps(QStringList fileNames);
    void addTilesets(QStringList fileNames);
    void addRules(QStringList fileNames);

    void setRulesSetLayerName(QString setLayerName);

    QString fileName() const { return mFileName; }
    QString rulesSetLayerName() const { return mRulesSetLayer; }

    QStringList maps() const { return QStringList(mMaps); }
    QStringList tilesets() const { return QStringList(mTilesets); }
    QStringList rules() const { return QStringList(mRules); }

    void dumpInfo();

protected:

    QStringList mMaps;
    QStringList mTilesets;
    QStringList mRules;

    QString mRulesSetLayer;
    QString mFileName;
};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // PROJECT_H
