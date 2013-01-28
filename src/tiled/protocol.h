/*
 * protocol.h
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
 *
 * This file is part of Tiled and tiled server.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
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

#ifndef TILEDPROTOCOL_H
#define TILEDPROTOCOL_H

#include <QtGlobal>
#include <QByteArray>
#include <QTcpSocket>
#include <QObject>
#include <QThread>
#include <QSemaphore>

/**
 * For networking we exchange messages.
 * Messages have the following format:
 * <id> <length> [<information>]
 *
 * The <id> is always 2 bytes. The id is a basic information of the packet,
 * i.e. how the message bytes should be handled.
 *
 * After the id follows the length information for the current message.
 * The length is always 4 bytes, giving the number of bytes which are the
 * actual information.
 *
 * After id and length there are informations.
 * The meaning of these information depends on the id, so see documentation
 * below.
 */

#define QTCONNECTIONVERSION (QDataStream::Qt_4_6)
#define T_BYTES_ID 2
#define T_BYTES_LENGTH 4

enum PackageIDS {
TMSG_NONE                       = 0x0000,
TMSG_PING                       = 0x0001,
TMSG_PING_ANS                   = 0x0002,
TMSG_VERSION                    = 0x0003,
TMSG_VERSION_ANS                = 0x0004,
TMSG_INFORMATION                = 0x0005,
TMSG_INFORMATION_ANS            = 0x0006,

TMSG_CHAT                       = 0x0010,

TMSG_LOGIN_REQUEST              = 0x0020,
TMSG_LOGIN_ACK                  = 0x0021,
TMSG_LOGIN_ATTEMPT              = 0x0022,
TMSG_LOGIN_SUCCESS              = 0x0023,
TMSG_LOGIN_FAILURE              = 0x0024,

/*returns a list of projects*/
TMSG_REQUEST_PROJECTS           = 0x0030,
TMSG_REQUEST_PROJECTS_ANS       = 0x0031,

/*returns a list of files within one project*/
TMSG_REQUEST_PROJECTDATA        = 0x0032,
TMSG_REQUEST_PROJECTDATA_ANS    = 0x0033,

/*returns a certain file, being specified by project and name*/
TMSG_REQUEST_MAP                = 0x0034,
TMSG_REQUEST_MAP_ANS            = 0x0035,

/*returns metadata about a project or file*/
TMSG_REQUEST_METADATA           = 0x0036,
TMSG_REQUEST_METADATA_ANS       = 0x0037,

/*start or end the notification of changes*/
TMSG_REQUEST_OPENFILE           = 0x0038,
TMSG_REQUEST_CLOSEFILE          = 0x0039,

TMSG_CHANGETILES                = 0x0040, // change tiles   <layerindex>, [<x 2 bytes>, <y 2 bytes>, <gid 4 bytes>]*
TMSG_RENAMELAYER                = 0x0050, // layer renaming <index 2 byte> <QString>
TMSG_MOVELAYER                  = 0x0051, // layer moving   <oldindex 2 byte> <newindex 2 byte>
TMSG_DELETELAYER                = 0x0052, // layer delete   <index 2 byte>
TMSG_ADDLAYER                   = 0x0053, // layer add      <index 2 byte> <type of layer 2 byte>
TMSG_LAYERCHANGED               = 0x0054  // layer changed opacity or visibility <1 byte visibility> <4 byte opacity float>
};

/**
 * This class represents a message. Length of the message can be get/set by
 * setting/getting the length of the data byte array.
 */
class TiledSocket;

class TiledMessage {
public:
    TiledMessage(quint16 id);
    TiledMessage();

    ~TiledMessage();

    int id() const { return mID; }
    QByteArray *data() { return &mInfo; }

    void setData(QByteArray newInfo);

    void dump();

private:
    quint16 mID;
    qint32 mLength;
    QByteArray mInfo;
};

/**
 * This class handles the messages.
 */
class TiledSocket : public QObject
{
    Q_OBJECT;
public:
    TiledSocket(QTcpSocket *socket, QObject *parent);
    ~TiledSocket();

signals:
    void handleMessage(TiledSocket *s, TiledMessage mes);
    void error(TiledSocket *s, QTcpSocket::SocketError socketError);

public slots:
    void sendMessage(TiledMessage mes);

private slots:
    void readData();

private:
    void throwMessage(TiledMessage mes);
    QTcpSocket *mTcpSocket;

    enum inState {
        readID,
        readLength,
        readInformation
    };
    inState mInState;

    /**
     * mInMessage is used to store all read data, which are not completed.
     * Once a complete message is arrived this array will be used in
     * handleMessage, and mInMessage gets a new byte array to be filled.
     */
    TiledMessage mInMessage;

    /**
     * Indicates the position which can be written next in mInMessage
     */
    int mInRead;

    /**
     * Indicates the last byte which can be put into mInMessage
     */
    int mInReadEnd;

    /**
     * Since we do need to make sure tiled messages are going over wire in
     * series and not interleaved somehow (this method is not reenetrant)
     * acquire this token just before sending a message.
     * TODO: do we need this?
     */
    QSemaphore mToken;
};

#endif // PROTOCOL_H


