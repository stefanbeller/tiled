/*
 * projectdock.h
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef PROJECTDOCK_H
#define PROJECTDOCK_H

#include <QString>
#include <QDockWidget>
#include <QTreeWidget>

namespace Tiled {
namespace Internal {

class Project;
class ProjectTreeModel;

/**
 * A QXmlStreamReader based reader for the MapEditorProject format.
 */
class ProjectDock  : public QDockWidget
{
    Q_OBJECT
public:
    ProjectDock(ProjectTreeModel *m, QWidget *parent = 0);
    ~ProjectDock();
protected:
    void changeEvent(QEvent *e);

signals:
    void openFile(QString file);

private slots:
    void doubleClick(const QModelIndex &index);

private:
    void retranslateUi();
    QTreeView *mTreeView;
    ProjectTreeModel *mProjectTreeModel;

    QAction *actionNewProject();
    QAction *actionNewMap();
    QAction *actionAddMap();
};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // PROJECTDOCK_H
