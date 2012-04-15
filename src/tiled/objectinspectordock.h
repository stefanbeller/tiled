/*
 * objectinspector.h
 * Copyright 2012, Nikola Ivanov <nikola.n.ivanov@gmail.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef OBJECTINSPECTORDOCK_H
#define OBJECTINSPECTORDOCK_H

#include "mapdocument.h"
#include "mapobject.h"
#include "objecttypesmodel.h"

#include <QModelIndex>
#include <QDockWidget>

namespace Ui {
    class ObjectInspectorDock;
}

namespace Tiled
{

namespace Internal
{

class PropertiesModel;


class ObjectInspectorDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectInspectorDock(QWidget *parent = 0);
    ~ObjectInspectorDock();
    void setMapDocument(MapDocument *);

private:
    void setMapObject(MapObject *);
    void changeMapObject();
    void resizeMapObject();
    void moveMapObject();
    void changeProperties();
    void clear();

private:
    Ui::ObjectInspectorDock *ui;
    MapDocument *mMapDocument;
    MapObject *mMapObject;
    ObjectTypesModel *mObjectTypesModel;
    PropertiesModel *mPropertiesModel;

private slots:
    void objectsChanged(const QList<MapObject*> &objects);
    void selectedObjectsChanged();
    void on_name_editingFinished();
    void type_editingFinished();
    void on_x_editingFinished();
    void on_y_editingFinished();
    void on_width_editingFinished();
    void on_height_editingFinished();
    void dataChanged(QModelIndex , QModelIndex );
    void layerChanged(int);
    void deleteSelectedProperties();
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTINSPECTORDOCK_H
