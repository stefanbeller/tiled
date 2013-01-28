/*
 * tiledserver.cpp
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "tiledserver.h"
#include "protocol.h"

#include "map.h"
#include "mapwriter.h"
#include "mapreader.h"

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QTcpServer>
#include <QApplication>

#include <QtNetwork>
#include <QDataStream>
#include <QTextStream>

TiledServer::TiledServer(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<TiledMessage>("TiledMessage");
    readConfiguration();
    startListening();
}

void TiledServer::readConfiguration()
{
    QSettings setting;

    if (!QFile(setting.fileName()).exists()) {
        qWarning()<<"no config file found at "<<setting.fileName();
    }

    mPort = setting.value("Port", 2751).toInt();
    QStringList maplist = setting.value("Maps", QStringList()).toStringList();

    if (maplist.isEmpty()) {
        qWarning()<<"No Maps to load.";
        exit(1); // TODO: some qt based solution?
    }

    MapReader r;
    foreach (QString s, maplist) {
        qDebug()<<s;
        Map *m = r.readMap(s);
        if (m) {
            mMaps.append(m);
            mMapNames.append(s);
        } else {
            qWarning()<<"Map not found: " << s;
        }
    }
}

void TiledServer::startListening()
{
    mTcpServer = new QTcpServer(0);
    int port = mPort;
    mTcpServer->listen(QHostAddress::Any, port);
    connect(mTcpServer, SIGNAL(newConnection()), this, SLOT(newClient()));
    QThread *TcpServerThread = new QThread();
    mTcpServer->moveToThread(TcpServerThread);
    TcpServerThread->start();
}

void TiledServer::newClient()
{
    qDebug()<<__FUNCTION__;
    QTcpSocket *socket = mTcpServer->nextPendingConnection();
    TiledSocket *connection = new TiledSocket(socket, 0);

    mConnections.append(connection);

    connect(connection, SIGNAL(handleMessage(TiledSocket*,TiledMessage)),
            this, SLOT(handleMessageFromNet(TiledSocket*,TiledMessage)));


}

//void TiledServer::messageToAll(ClientConnection *sender, TiledMessage message)
//{
//    sender = sender;
//    foreach (ClientConnection *c, mConnections ) {
//        c->sendMessageServerSlot(message);
//    }
//}

void TiledServer::handleMessageFromNet(TiledSocket *s, TiledMessage mes)
{
    qDebug()<<"ClientConnection::"<<__FUNCTION__<<mes.id();
    switch (mes.id()) {
    case TMSG_REQUEST_PROJECTS: {
        TiledMessage t(TMSG_REQUEST_PROJECTS_ANS);
        QStringList list;
        QByteArray bytearray;
        QDataStream stream( &bytearray, QIODevice::WriteOnly );
        list<<QString(tr("default"));
        stream << list;
        t.setData(bytearray);
        s->sendMessage(t);
        } break;
    case TMSG_REQUEST_PROJECTDATA: {
        TiledMessage t(TMSG_REQUEST_PROJECTDATA_ANS);
        QStringList list;
        QByteArray bytearray;
        QDataStream stream( &bytearray, QIODevice::WriteOnly );
        foreach (QString s, mMapNames) {
            list<<s;
        }
        stream << list;
        t.setData(bytearray);
        s->sendMessage(t);
        } break;
    case TMSG_REQUEST_MAP: {
        QString filename;
        QDataStream stream( mes.data(), QIODevice::ReadOnly );
        stream >> filename;
        qDebug()<<"sending map"<<filename;
        //QByteArray block;
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        //buf.setBuffer(&block);
        TiledMessage t(TMSG_REQUEST_MAP_ANS);
        qDebug()<<"constructed answer";
        MapReader *r = new MapReader();
        //r->setLazy(true);
        Map *map = r->readMap(filename);
        Q_ASSERT(map);
        qDebug()<<"read map";
        MapWriter *w = new MapWriter();
        w->writeMap(map, &buf, QLatin1String("."));
        qDebug()<<"block written";
        buf.close();
        qDebug()<<"set Length to "<<buf.size();
        t.setData(buf.data());
        s->sendMessage(t);
        qDebug()<<"map sent";
        qDebug()<<t.data()->data();
        }
        break;

    case TMSG_CHANGETILES: {
        foreach (TiledSocket *c, mConnections ) {
            if (c != s)
                c->sendMessage(mes);
        }
    } break;

    case TMSG_RENAMELAYER: {
        foreach (TiledSocket *c, mConnections ) {
            if (c != s)
                c->sendMessage(mes);
        }
    } break;

    case TMSG_MOVELAYER: {
        foreach (TiledSocket *c, mConnections ) {
            if (c != s)
                c->sendMessage(mes);
        }
    } break;

    case TMSG_DELETELAYER: {
        foreach (TiledSocket *c, mConnections ) {
            if (c != s)
                c->sendMessage(mes);
        }
    } break;

    case TMSG_ADDLAYER: {
        foreach (TiledSocket *c, mConnections ) {
            if (c != s)
                c->sendMessage(mes);
        }
    } break;

    default:
        TiledMessage m(TMSG_PING);
        s->sendMessage(m);
        break;
    }
}

