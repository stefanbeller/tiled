/*
 * Python Tiled Plugin
 * Copyright 2012, Samuli Tuomola <samuli@tuomola.net>
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

#ifndef PYTHONPLUGIN_H
#define PYTHONPLUGIN_H

#include <QtCore/qglobal.h>

#include "mapwriterinterface.h"
#include "mapreaderinterface.h"

#include <QObject>
#include <Python.h>

namespace Tiled {
class Map;
}

namespace Python {

class Q_DECL_EXPORT PythonPlugin
        : public QObject
        , public Tiled::MapReaderInterface
        , public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapReaderInterface Tiled::MapWriterInterface)

public:
    PythonPlugin();
    ~PythonPlugin();

    // MapReaderInterface
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);

    // Both interfaces
    QStringList nameFilters() const;
    QString errorString() const;

private:
    void handleError() const;
    PyObject *findPluginSubclass(PyObject *pmod);
    PyObject *checkFunction(PyObject *pcls, const char *fun) const;
    bool checkFileSupport(PyObject* cls, char *file) const;
    void reloadModules();
    QString mError;
    uint lastReload;
};

// Class exposed for python scripts to extend
class PythonScript {
public:
    // perhaps provide default that throws NotImplementedError
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
};

} // namespace Python

PyMODINIT_FUNC inittiled(void);
extern int _wrap_convert_py2c__Tiled__Map(PyObject *obj, Tiled::Map *map);
extern PyObject* _wrap_convert_c2py__Tiled__Map_const(Tiled::Map const *cvalue);

#endif // PYTHONPLUGIN_H
