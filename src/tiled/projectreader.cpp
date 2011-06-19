/*
 * projectreader.cpp
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


#include "projectreader.h"
#include "project.h"

#include "objectgroup.h"
#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QXmlStreamReader>
#include <QStringList>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class ProjectReaderPrivate
{
    Q_DECLARE_TR_FUNCTIONS(ProjectReader)

public:
    ProjectReaderPrivate(ProjectReader *projectReader)
        : mProjectReader(projectReader)
    {}

    Project *readProject(QIODevice *device, const QString &path);

    bool openFile(QFile *file);

    QString errorString() const;

private:
    bool readNextStartElement();
    void readUnknownElement();
    void skipCurrentElement();

    Project *readProject();

    QStringList readStringList();

    ProjectReader *mProjectReader;

    QString mError;
    QString mPath;

    Project *mProject;
    QXmlStreamReader xml;
};

} // namespace Internal
} // namespace Tiled

Project *ProjectReaderPrivate::readProject(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath = path;
    Project *project = 0;

    xml.setDevice(device);

    if (readNextStartElement() && xml.name() == "project") {
        project = readProject();
    } else {
        xml.raiseError(tr("Not a project file."));
    }
    return project;
}


QString ProjectReaderPrivate::errorString() const
{
    if (!mError.isEmpty()) {
        return mError;
    } else {
        return tr("%3\n\nLine %1, column %2")
                .arg(xml.lineNumber())
                .arg(xml.columnNumber())
                .arg(xml.errorString());
    }
}

bool ProjectReaderPrivate::openFile(QFile *file)
{
    if (!file->exists()) {
        mError = tr("File not found: %1").arg(file->fileName());
        return false;
    } else if (!file->open(QFile::ReadOnly | QFile::Text)) {
        mError = tr("Unable to read file: %1").arg(file->fileName());
        return false;
    }

    return true;
}

bool ProjectReaderPrivate::readNextStartElement()
{
    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement())
            return false;
        else if (xml.isStartElement())
            return true;
    }
    return false;
}

void ProjectReaderPrivate::readUnknownElement()
{
    qDebug() << "Unknown element (fixme):" << xml.name();
    skipCurrentElement();
}

void ProjectReaderPrivate::skipCurrentElement()
{
    while (readNextStartElement())
        skipCurrentElement();
}

Project *ProjectReaderPrivate::readProject()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "project");

    mProject = new Project(mPath);

    while (readNextStartElement()) {
        if (xml.name() == "maps")
            mProject->addMaps(readStringList());
        else if (xml.name() == "tilesets")
            mProject->addTilesets(readStringList());
        else if (xml.name() == "rules")
            mProject->addRules(readStringList());
        else
            readUnknownElement();
    }

    // Clean up in case of error
    if (xml.hasError()) {
        delete mProject;
        mProject = 0;
    }
    mProject->dumpInfo();
    return mProject;
}

QStringList ProjectReaderPrivate::readStringList()
{
    Q_ASSERT(xml.isStartElement());

    QStringList ret;

    ret.append(xml.readElementText().split(QLatin1String("\n")));
    return ret;
}


ProjectReader::ProjectReader()
    : d(new ProjectReaderPrivate(this))
{
}

ProjectReader::~ProjectReader()
{
    delete d;
}

Project *ProjectReader::readProject(QIODevice *device, const QString &path)
{
    return d->readProject(device, path);
}

Project *ProjectReader::readProject(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return 0;

    return readProject(&file, QFileInfo(fileName).absolutePath());
}

QString ProjectReader::errorString() const
{
    return d->errorString();
}
