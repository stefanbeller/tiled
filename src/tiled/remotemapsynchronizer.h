/*
 * remotemapsynchronizer.h
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
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
#ifndef REMOTEMAPSYNCHRONIZER_H
#define REMOTEMAPSYNCHRONIZER_H

#include <QObject>
#include <QRegion>
#include <QTcpSocket>
#include <QSemaphore>
#include <QMutex>
#include <QThread>

#include "protocol.h"
#include "mainwindow.h"
#include "gidmapper.h"

namespace Tiled {

class Layer;
class Map;
class MapObject;
class MapRenderer;
class Tileset;

namespace Internal {
class MapDocument;

class RemoteMapSynchronizer : public QObject
{
    Q_OBJECT
public:
    RemoteMapSynchronizer(QObject *parent = 0);
    ~RemoteMapSynchronizer();

    //MapDocument *mapDocument();

    QString errorString();

    void connectToServer(const QString &server, const int port);

signals:
    void newMapDocument(MapDocument*);
    void listOfProjects(QStringList);
    void listOfMaps(QStringList);

    void sendMessage(TiledMessage);

public slots:
    /**
     * When data have been arrived from server, this slot is called.
     */
    void handleMessage(TiledSocket *sender,TiledMessage message);

    void remoteRequestProject(QString project);
    void remoteRequestMap(QString map);

private slots:
    /**
     * sends all changed stuff to the server.
     */
    void regionEdited(const QRegion &region, Layer *layer);

    void layerChanged(int);
    void layerAdded(int);
    void layerRemoved(int);
    void layerRenamed(int);

    /**
     * In case of broken internet, the problem is repaired here:
     */
    void setErrorString(QAbstractSocket::SocketError socketError);



private:
    MapDocument *mMapDocument;
    QTcpSocket *mTcpSocket;
    TiledSocket *mTiledSocket;
    QString mError;
    QString mServer;
    int mPort;
    GidMapper mGidMapper;
    int mProcessingID;
};

}
}
#endif // REMOTEMAPSYNCHRONIZER_H
