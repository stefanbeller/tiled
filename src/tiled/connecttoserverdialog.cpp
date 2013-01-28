/*
 * connecttoserverdialog.cpp
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
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

#include "connecttoserverdialog.h"
#include "ui_connecttoserverdialog.h"

#include "preferences.h"
#include "remotemapsynchronizer.h"

#include "mainwindow.h"

#include <QSettings>

using namespace Tiled::Internal;

ConnectToServerDialog::ConnectToServerDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::connectNetworkDialog)
{
    mUi->setupUi(this);
    setupServerList();
    setupUserList();
}

ConnectToServerDialog::~ConnectToServerDialog()
{
    delete mUi;
}


void ConnectToServerDialog::getConnection(QString &Server, int &Port)
{
    if (exec() != QDialog::Accepted)
        return ;

    QString server = mUi->serverBox->currentText();
    QString user = mUi->userBox->currentText();
    int port = mUi->portBox->value();

    addServerToList(server);
    addUserToList(user);

    Server = server;
    Port = port;

    return;
}

void ConnectToServerDialog::setupServerList()
{
    QSettings *s = Preferences::instance()->settings();
    QStringList servers = s->value(QLatin1String("Net/Servers")).toStringList();
    mUi->serverBox->addItems(servers);
    mUi->serverBox->setCurrentIndex(mUi->serverBox->count() - 1);
}

void ConnectToServerDialog::addServerToList(QString server)
{
    QSettings *s = Preferences::instance()->settings();
    QStringList servers = s->value(QLatin1String("Net/Servers")).toStringList();
    if (servers.contains(server))
        servers.removeAll(server);
    servers.append(server);
    s->setValue(QLatin1String("Net/Servers"), servers);
}

void ConnectToServerDialog::setupUserList()
{
    QSettings *s = Preferences::instance()->settings();
    QStringList users = s->value(QLatin1String("Net/Users")).toStringList();
    mUi->userBox->addItems(users);
    mUi->userBox->setCurrentIndex(mUi->userBox->count() - 1);
}

void ConnectToServerDialog::addUserToList(QString user)
{
    QSettings *s = Preferences::instance()->settings();
    QStringList users = s->value(QLatin1String("Net/Users")).toStringList();
    if (users.contains(user))
        users.removeAll(user);
    users.append(user);
    s->setValue(QLatin1String("Net/Users"), users);
}
