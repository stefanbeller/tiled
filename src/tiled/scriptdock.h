/*
 * scriptdock.h
 * Copyright 2011, Christophe Conceição <christophe.conceicao@gmail.com>
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

#ifndef SCRIPTDOCK_H
#define SCRIPTDOCK_H

#include "mapdocument.h"
#include "scripteditor.h"
#include "scripthighlighter.h"

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>


namespace Tiled {
namespace Internal {

/**
 * A dock widget with lua scripting features.
 */
class ScriptDock : public QDockWidget
{
    Q_OBJECT

public:
    ScriptDock(QWidget *parent = 0);

    /**
     * Sets the map for which the layers should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

    QString fileName() const { return mFileName; }
    void newScript();
    bool openScript(const QString &fileName);
    bool saveScript(const QString &fileName);

public slots:
    void runScript();

protected:
    void changeEvent(QEvent *e);

private:
    QString mFileName;
    ScriptEditor *mScriptArea;
    ScriptHighlighter *highlighter;
    QPlainTextEdit *mOutputArea;
    MapDocument *mMapDocument;

    void retranslateUi();

private slots:
    void scriptChanged();
};

} // namespace Internal
} // namespace Tiled

#endif // SCRIPTDOCK_H
