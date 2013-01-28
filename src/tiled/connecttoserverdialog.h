/*
 * connecttoserverdialog.h
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

#ifndef CONNECTNETWORKDIALOG_H
#define CONNECTNETWORKDIALOG_H

#include <QDialog>

#include "mainwindow.h"

namespace Ui {
    class connectNetworkDialog;
}


namespace Tiled {
namespace Internal {

class MapDocument;

class ConnectToServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectToServerDialog(QWidget *parent = 0);
    ~ConnectToServerDialog();

    /**
     * Shows the dialog and returns the created map. Returns 0 if the dialog
     * was cancelled.
     */
    void getConnection(QString &Server, int &Port);

private:
    void setupServerList();
    void addServerToList(QString server);
    void setupUserList();
    void addUserToList(QString user);
    Ui::connectNetworkDialog *mUi;
};

} // namespace Internal
} // namespace Tiled

#endif // CONNECTNETWORKDIALOG_H
