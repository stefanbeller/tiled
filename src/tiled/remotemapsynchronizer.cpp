/*
 * remotemapsynchronizer.cpp
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

#include "remotemapsynchronizer.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapreader.h"
#include "mapdocument.h"
#include "objectgroup.h"
#include "tile.h"
#include "tileset.h"
#include "tilelayer.h"
#include "protocol.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QDataStream>
#include <QBuffer>

using namespace Tiled;
using namespace Tiled::Internal;

RemoteMapSynchronizer::RemoteMapSynchronizer(QObject *parent )
    : QObject(parent)
    , mMapDocument(0)
    , mServer(QString())
    , mPort(0)
{

}

void RemoteMapSynchronizer::connectToServer(const QString &server, const int port)
{
    mServer = server;
    mPort = port;
    mTcpSocket = new QTcpSocket(0);
    mTcpSocket->connectToHost(mServer, mPort);

    qDebug()<<"connecting to "<<mServer<<"at port"<<mPort;
    if(!mTcpSocket->waitForConnected(5000)) {
        qDebug()<<"not connected!";
        return;
    }
    qDebug()<<"connected!";

    mTiledSocket = new TiledSocket(mTcpSocket, 0);

    connect(mTiledSocket, SIGNAL(handleMessage(TiledSocket*,TiledMessage)),
            this, SLOT(handleMessage(TiledSocket*,TiledMessage)));

    connect(this, SIGNAL(sendMessage(TiledMessage)),
            mTiledSocket,SLOT(sendMessage(TiledMessage)));

    TiledMessage m(TMSG_REQUEST_PROJECTS);
    emit sendMessage(m);

    QThread *TiledSocketThread = new QThread();
    mTiledSocket->moveToThread(TiledSocketThread);
    mTcpSocket->moveToThread(TiledSocketThread);
    TiledSocketThread->start();
}

void RemoteMapSynchronizer::remoteRequestProject(QString project)
{
    TiledMessage t(TMSG_REQUEST_PROJECTDATA);
    QDataStream stream( t.data(), QIODevice::WriteOnly );
    stream << project;
    sendMessage(t);
}

void RemoteMapSynchronizer::remoteRequestMap(QString map)
{
    TiledMessage t(TMSG_REQUEST_MAP);
    QDataStream stream( t.data(), QIODevice::WriteOnly );
    stream << map;
    sendMessage(t);
}

RemoteMapSynchronizer::~RemoteMapSynchronizer()
{

}

void RemoteMapSynchronizer::handleMessage(TiledSocket *sender, TiledMessage m)
{
    mProcessingID = m.id();
    qDebug() << "recv:"<<m.id()<< "  "<<m.data();
    qDebug() << m.data()->length();
    qDebug() << m.data()->data();
    QDataStream in(m.data(), QIODevice::ReadOnly);
    switch(m.id()) {
    case TMSG_REQUEST_PROJECTS_ANS: {
        QStringList projects;
        in >> projects ;
        emit listOfProjects(projects);
    } break;
    case TMSG_REQUEST_PROJECTDATA_ANS: {
        QStringList maps;
        in >> maps ;
        emit listOfMaps(maps);
    } break;
    case TMSG_REQUEST_MAP_ANS: {
        QBuffer buf;
        buf.setBuffer(m.data());
        buf.open(QIODevice::ReadOnly);

        MapReader *r = new MapReader();
        //r->setLazy(false);
        Map *map = r->readMap(&buf, QLatin1String("."));
        if (!map) {
            qDebug() << "error receiving map";
            qDebug() << r->errorString();
            break;
        }
        qDebug()<<"map->tilesets():"<< map->tilesets();
        foreach (const Tileset *ts, map->tilesets()) {
            qDebug()<<ts->name()<<ts->fileName()<< ts->imageSource()<< ts->columnCount();
        }

        buf.close();
        qDebug()<<"|"<<r->errorString()<<"|";
        Q_ASSERT(map!=0);
        delete r;
        qDebug()<<"map exists!";
        mMapDocument = new MapDocument(map,QString(),this);
        qDebug()<<"asdf";
        connect(mMapDocument, SIGNAL(regionEdited(QRegion,Layer*)),
                this, SLOT(regionEdited(QRegion,Layer*)));

        connect(mMapDocument, SIGNAL(layerAdded(int)),
                this, SLOT(layerAdded(int)));
        connect(mMapDocument, SIGNAL(layerChanged(int)),
                this, SLOT(layerChanged(int)));
        connect(mMapDocument, SIGNAL(layerRemoved(int)),
                this, SLOT(layerRemoved(int)));
        connect(mMapDocument->layerModel(), SIGNAL(layerRenamed(int)),
                this, SLOT(layerRenamed(int)));

        qDebug()<<"connected mapdocument to me";

        mGidMapper.clear();
        uint firstGid = 1;
        foreach (Tileset *tileset, map->tilesets()) {
            mGidMapper.insert(firstGid, tileset);
            firstGid += tileset->tileCount();
        }
        qDebug ()<<"new Map signal";
        emit newMapDocument(mMapDocument);

        } break;
    case TMSG_CHANGETILES: {
        in.setVersion(QTCONNECTIONVERSION);

        quint16 layer;
        qint32 length;
        in >> layer;
        ///qDebug()<<"length="<<m.length();
        qDebug()<<"layer="<<layer;
        length = m.data()->length();
        Q_ASSERT(length%8 == 0);
        bool ok;
        QRegion changedRegion;
        while (length > 0) {
            quint16 x;
            quint16 y;
            quint32 gid;
            in >> x;
            in >> y;
            in >> gid;
            qDebug()<<"x:"<<x<<"y:"<<y<<"gid:"<<gid;
            length-=8;
            ok=false;
            Cell c = mGidMapper.gidToCell(gid,ok);
            Q_ASSERT(ok);
            mMapDocument->map()->layerAt(layer)->asTileLayer()->setCell(x,y,c);
            changedRegion+=QRect(x,y,1,1);
        }
        mMapDocument->emitRegionChanged(changedRegion);
    } break;

    case TMSG_RENAMELAYER: {
        quint16 index;
        QString newName;
        in >> index;
        in >> newName;
        qDebug()<<"renaming layer"<<index<<"to:"<<newName;
        mMapDocument->layerModel()->renameLayer(index, newName);
    } break;

    case TMSG_MOVELAYER: {
        quint16 oldindex;
        quint16 newindex;
        in >> oldindex;
        in >> newindex;

    } break;

    case TMSG_DELETELAYER: {
        quint16 index;
        in >> index;
        mMapDocument->removeLayer(index);
    } break;

    case TMSG_ADDLAYER: {
        quint16 index;
        quint16 type;
        in >> index;
        in >> type;
        if (type == 1) {
            mMapDocument->addLayer(Layer::TileLayerType);
        } else if (type == 2) {
            mMapDocument->addLayer(Layer::ObjectGroupType);
        } else {
            qDebug() << "error in "<<__FILE__<<__FUNCTION__<<__LINE__;
        }
    } break;
    default:
        m.dump();
        qDebug() << "error: something strange happens here in file"<< __FILE__<<__FUNCTION__<<__LINE__;
        break;
    }
    mProcessingID = 0;
}

//MapDocument *RemoteMapSynchronizer::mapDocument()
//{
//    return mMapDocument;
//}

void RemoteMapSynchronizer::setErrorString(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        mError += tr("The host was not found. Please check the "
                     "host name and port settings.\n");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        mError += tr("The connection was refused by the peer. "
                     "Make sure the fortune server is running, "
                     "and check that the host name and port "
                     "settings are correct.\n");
        break;
    default:
        mError += tr("The following error occurred: %1.\n").arg(
                mTcpSocket->errorString());
    }
    qDebug() << mError;
}

QString RemoteMapSynchronizer::errorString()
{
    QString ret(mError);
    mError = QString();
    return ret;
}

void RemoteMapSynchronizer::regionEdited(const QRegion &region, Layer *layer)
{
    qDebug()<<"RemoteMapSynchronizer::"<<__FUNCTION__;
    TileLayer *tl = layer->asTileLayer();
    if (!tl)
        return;

    TiledMessage mes(TMSG_CHANGETILES);
    QDataStream out(mes.data(), QIODevice::WriteOnly);
    quint16 layernumber = quint16(layer->map()->layers().indexOf(layer));
    out << layernumber;
    int n = 0; // number of tiles
    foreach (QRect r, region.rects()) {
        for (quint16 x = r.left(); x <= r.right(); x++)
            for (quint16 y = r.top(); y <= r.bottom(); y++) {
                n++;
                const quint32 gid = mGidMapper.cellToGid(tl->cellAt(x, y));
                out << x;
                out << y;
                out << gid;
                qDebug()<<x<<y<<gid;
            }
    }
    qDebug()<<"regionEdited: mes.info()->size()"<<mes.data()->length();
    qDebug()<<"regionEdited: should be 2+8*n="<<2+8*n;

    emit sendMessage(mes);
    return;
}

void RemoteMapSynchronizer::layerChanged(int layerindex)
{
//    qDebug()<<"RemoteMapSynchronizer::"<<__FUNCTION__;
//    TiledMessage mes(TMSG_DELETELAYER);
//    mes.setData(QByteArray(2, 0));
//    QDataStream out(mes.data(), QIODevice::WriteOnly);
//    out << layerindex;
//    emit sendMessage(mes);
}

void RemoteMapSynchronizer::layerAdded(int layerindex)
{
    if (mProcessingID)
        return;
    qDebug()<<"RemoteMapSynchronizer::"<<__FUNCTION__;
    TiledMessage mes(TMSG_ADDLAYER);
    mes.setData(QByteArray(4, 0));
    QDataStream out(mes.data(), QIODevice::WriteOnly);
    quint16 type = 0;
    if (mMapDocument->map()->layerAt(layerindex)->asTileLayer()) {
        type = 1;
    } else if (mMapDocument->map()->layerAt(layerindex)->asObjectGroup()) {
        type = 2;
    }
    out << (quint16)layerindex;
    out << type;
    emit sendMessage(mes);
}

void RemoteMapSynchronizer::layerRemoved(int layerindex)
{
    if (mProcessingID)
        return;
    TiledMessage mes(TMSG_DELETELAYER);
    QDataStream out(mes.data(), QIODevice::WriteOnly);
    out << (quint16)layerindex;
    emit sendMessage(mes);
}

void RemoteMapSynchronizer::layerRenamed(int layerindex)
{
    if (mProcessingID)
        return;
    TiledMessage mes(TMSG_RENAMELAYER);
    QDataStream out(mes.data(), QIODevice::WriteOnly);
    out << (quint16)layerindex;
    out << mMapDocument->map()->layerAt(layerindex)->name();
    emit sendMessage(mes);
}


