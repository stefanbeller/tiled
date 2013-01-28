/*
 * tiledserver.h
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 *
 * This file is part of tiled server.
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

#ifndef TILEDSERVER_H
#define TILEDSERVER_H

#include "map.h"
#include "protocol.h"

#include <QThread>
#include <QTcpSocket>
#include <QStringList>
#include <QTcpServer>

class TiledMessage;

using namespace Tiled;

class TiledServer : public QObject
{
    Q_OBJECT

public:
    TiledServer(QObject *parent = 0);

protected slots:
    void newClient();
    void handleMessageFromNet(TiledSocket *s, TiledMessage mes);

private:
    void readConfiguration();
    void startListening();

    QList<QString> mMapNames;
    QList<Map*> mMaps;
    QList<TiledSocket*> mConnections;
    QTcpServer *mTcpServer;

    /**
     * At this port the server listens for collaborative clients.
     */
    int mPort;

};

#endif
