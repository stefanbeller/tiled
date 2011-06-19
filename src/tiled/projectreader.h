/*
 * projectreader.h
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

#ifndef PROJECTREADER_H
#define PROJECTREADER_H


#include <QString>
#include <QIODevice>

#include "project.h"

class QFile;

namespace Tiled {
class Map;
class Tileset;

namespace Internal {

class ProjectReaderPrivate;
/**
 * A fast QXmlStreamReader based reader for the TILED Project format
 */
class ProjectReader
{
public:
    ProjectReader();
    ~ProjectReader();

    Project *readProject(QIODevice *device, const QString &path = QString());

    Project *readProject(const QString &fileName);

    /**
     * Returns the error message for the last occurred error.
     */
    QString errorString() const;

private:
    friend class ProjectReaderPrivate;
    ProjectReaderPrivate *d;
};

} // namespace Internal
} // namespace Tiled

#endif // PROJECTREADER_H
