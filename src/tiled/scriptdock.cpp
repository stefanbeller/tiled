/*
 * scriptdock.cpp
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

#include "scriptdock.h"
#include "luabinds.h"

#include <QEvent>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

ScriptDock::ScriptDock(QWidget *parent)
    : QDockWidget(parent),
    mFileName(tr("untitled.lua")),
    mMapDocument(0)
{
    setObjectName(QLatin1String("scriptViewDock"));

    mScriptArea = new ScriptEditor(this);
    mScriptArea->setLineWrapMode(QPlainTextEdit::NoWrap);
    connect(mScriptArea, SIGNAL(textChanged()),
            SLOT(scriptChanged()));

    mOutputArea = new QPlainTextEdit(this);
    mOutputArea->setReadOnly(true);

    highlighter = new ScriptHighlighter(mScriptArea->document());


    QToolBar *buttonContainer = new QToolBar(this);
    buttonContainer->setFloatable(false);
    buttonContainer->setMovable(false);
    buttonContainer->setIconSize(QSize(16, 16));

    QAction *runAction = new QAction(this);
    runAction->setShortcut(tr("Ctrl+Shift+R"));
    runAction->setIcon(
            QIcon(QLatin1String(":/images/16x16/script_run.png")));

    buttonContainer->addAction(runAction);

    connect(runAction, SIGNAL(triggered()),
            SLOT(runScript()));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);

    QSplitter *splitter = new QSplitter(Qt::Vertical, widget);
    splitter->addWidget(mScriptArea);
    splitter->addWidget(mOutputArea);
    layout->addWidget(splitter);
    layout->addWidget(buttonContainer);

    QList<int> sizeList;
    sizeList << 500 << 100;
    splitter->setSizes(sizeList);

    setWidget(widget);
    retranslateUi();
}

void ScriptDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;
}

void ScriptDock::newScript()
{
    mScriptArea->clear();
    setWindowTitle(tr("untitled.lua"));
}

bool ScriptDock::openScript(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    mScriptArea->setPlainText(in.readAll());
    file.close();
    mFileName = fileName;

    setWindowTitle(QFileInfo(file).fileName());

    return true;
}

bool ScriptDock::saveScript(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << mScriptArea->toPlainText();
    file.close();
    mFileName = fileName;

    setWindowTitle(QFileInfo(file).fileName());

    return true;
}

void ScriptDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void ScriptDock::scriptChanged()
{
    setWindowTitle(QFileInfo(mFileName).fileName()+tr("*"));
}

void ScriptDock::retranslateUi()
{
    setWindowTitle(QFileInfo(mFileName).fileName());
}

static int luatiled_qtprint (lua_State *L) {
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_pushstring(L, "__scriptdock__");
    lua_gettable(L, LUA_REGISTRYINDEX);
    QTextEdit* scriptDockOutput = reinterpret_cast<QTextEdit*>(lua_touserdata(L, -1));
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        s = lua_tostring(L, -1);  /* get result */
        if (s == NULL)
            return luaL_error(L, LUA_QL("tostring") " must return a string to "
                              LUA_QL("print"));
        if (i>1) scriptDockOutput->insertPlainText(QObject::tr("\t"));
        scriptDockOutput->insertPlainText(QObject::tr(s));
        lua_pop(L, 1);  /* pop result */
    }
    scriptDockOutput->insertPlainText(QObject::tr("\n"));
    return 0;
}

void ScriptDock::runScript()
{
    mOutputArea->clear();

    /* the Lua interpreter */
    lua_State* L;

    /* initialize Lua */
    L = luaL_newstate();

    /* load Lua base libraries */
    luaL_openlibs(L);

    lua_pushstring(L, "__scriptdock__");
    lua_pushlightuserdata(L, reinterpret_cast<void*>(mOutputArea));
    lua_settable(L, LUA_REGISTRYINDEX);

    lua_pushstring(L, "__mapdocument__");
    lua_pushlightuserdata(L, reinterpret_cast<void*>(mMapDocument));
    lua_settable(L, LUA_REGISTRYINDEX);

    luaL_Reg base_override_funcs[] = {
      {"print", luatiled_qtprint},
      {NULL, NULL}
    };

    luaL_register(L, "_G", base_override_funcs);
    lua_pop(L, 1);
    registerLuaBinds(L, mMapDocument->map());

    /* eval and run */
    if(luaL_dostring(L, mScriptArea->toPlainText().toStdString().c_str()) != 0){
        mOutputArea->insertPlainText(tr(lua_tostring(L, -1)));
    }

    /* cleanup Lua */
    lua_close(L);
}
