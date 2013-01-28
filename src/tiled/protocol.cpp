/*
 * protocol.cpp
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


#include "protocol.h"
#include <QDebug>

TiledMessage::TiledMessage(quint16 id)
    : mID(id)
    , mInfo(QByteArray())
{

}

TiledMessage::TiledMessage()
    : mID(0)
    , mInfo(QByteArray())
{

}

TiledMessage::~TiledMessage()
{

}

void TiledMessage::setData(QByteArray newInfo)
{
    mInfo = newInfo;
}

void TiledMessage::dump()
{
    qDebug()<<"-------------------------";
    qDebug()<<"dump of message";
    qDebug()<<"Message( id ="<<id()<<", length ="<<data()->length()<<")";
    int length = data()->length();
    int i=0;
    while (length > 0) {
        qDebug() << (int)data()->at(i);
        i++;
        length--;
    }
    qDebug()<<"-------------------------";
}

//int TiledMessage::length() const
//{
//    return mInfo.length();
//}

TiledSocket::TiledSocket(QTcpSocket *socket, QObject *parent)
    : QObject(parent)
    , mTcpSocket(socket)
    , mInState(readID)
    , mInMessage(0)
    , mToken(1)
{
    connect (mTcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
}

TiledSocket::~TiledSocket()
{
    delete mTcpSocket;
}

void inline TiledSocket::throwMessage(TiledMessage mes)
{
    qDebug()<<"TiledSocket::"<<__FUNCTION__;
    qDebug()<<"Message( id ="<<mes.id()<<", length ="<<mes.data()->length()<<")";
    int length = mes.data()->length();
    int i=0;
    while (length>16) {
        qDebug()<<(int)mes.data()->at(i+0)<<(int)mes.data()->at(i+1)<<(int)mes.data()->at(i+2)
            <<(int)mes.data()->at(i+3)<<(int)mes.data()->at(i+4)<<(int)mes.data()->at(i+5)
            <<(int)mes.data()->at(i+6)<<(int)mes.data()->at(i+7)<<(int)mes.data()->at(i+8)
            <<(int)mes.data()->at(i+9)<<(int)mes.data()->at(i+10)<<(int)mes.data()->at(i+11)
            <<(int)mes.data()->at(i+12)<<(int)mes.data()->at(i+13)<<(int)mes.data()->at(i+14)
            <<(int)mes.data()->at(i+15);
        i+=16;
        length-=16;
    }
    qDebug()<<mes.data();
    emit handleMessage(this, mes);
}

void TiledSocket::readData()
{
    quint32 length;
    quint16 id;
    QDataStream in(mTcpSocket);
    in.setVersion(QTCONNECTIONVERSION);
    int len = mTcpSocket->bytesAvailable();
    while (len) {
        switch (mInState) {
        case readID:
            if (len < T_BYTES_ID)
                return;
            in >> id;
            mInMessage = TiledMessage(id);
            mInRead = 0;
            mInState = readLength;
            break;
        case readLength:
            if (len < T_BYTES_LENGTH)
                return;
            in >> length;
            if (length) {
                mInReadEnd = length;
                mInState = readInformation;
                mInMessage.setData(QByteArray(length, '\0'));
                qDebug()<<length<<mInMessage.data()->length();
                Q_ASSERT(length == mInMessage.data()->length());
            } else {
                throwMessage(mInMessage);
                mInState = readID;
            }
            break;

        case readInformation:
            int diff = mInReadEnd - mInRead;
            if (len >= diff) {
                QByteArray received(mTcpSocket->read(diff));
                mInMessage.data()->replace(mInRead, received.length(), received);
                throwMessage(mInMessage);
                mInState = readID;
            } else {
                QByteArray received(mTcpSocket->read(len));
                mInMessage.data()->replace(mInRead, received.length(), received);
                mInRead += len;
            }
            break;
        }
        len = mTcpSocket->bytesAvailable();
    }
}

void TiledSocket::sendMessage(TiledMessage mes){
    //qDebug()<<"TiledSocket sending Message"<<mes.id()<<"with length"<<mes.data()->length()<<"data:"<<mes.data()->data() ;
    //mToken.acquire(); see if we can do it without token

    int txbytes = T_BYTES_ID;
    QByteArray block(mes.data()->length() + T_BYTES_ID + T_BYTES_LENGTH, 0);
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QTCONNECTIONVERSION);

    out << (quint16)mes.id();

    out << (quint32)(mes.data()->length());
    txbytes += T_BYTES_LENGTH;

    txbytes += out.writeRawData(mes.data()->data(), mes.data()->length());

    int ret = mTcpSocket->write(block, txbytes);

    if (ret < 0) {
        // errorhandling in here!
        qDebug() << "Error in protocol.cpp line"<<__LINE__;
    }
    //mToken.release();
}


