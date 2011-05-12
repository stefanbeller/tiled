/*
 * luabinds.h
 * Copyright 2011, Christophe Conceicao <christophe.conceicao@gmail.com>
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

#ifndef LUABINDS_H
#define LUABINDS_H

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "tilepainter.h"
#include "mapdocument.h"
#include "map.h"
#include "layer.h"
#include "tilelayer.h"
#include "tilesetmanager.h"
#include "tileset.h"
#include "tile.h"

using namespace Tiled;
using namespace Tiled::Internal;


#define CREATEBIND(classname) \
static int luatiled_create##classname (lua_State *L, classname *map) { \
    lua_newtable(L); \
    lua_getglobal(L, #classname); \
    lua_setmetatable(L, -2); \
    lua_pushstring(L, "__self"); \
    classname **luaMap = (classname **)lua_newuserdata(L, sizeof(classname *)); \
    *luaMap = map; \
    luaL_getmetatable(L, "Tiled."#classname); \
    lua_setmetatable(L, -2); \
    lua_rawset(L, -3); \
    return 1; \
} \
classname* check##classname(lua_State* L, int index) { \
  void* ud = 0; \
  luaL_checktype(L, index, LUA_TTABLE); \
  lua_pushstring(L, "__self"); \
  lua_rawget(L, index); \
  ud = luaL_checkudata(L, -1, "Tiled."#classname); \
  luaL_argcheck(L, ud != 0, 1, "`Tiled."#classname"' expected"); \
  return *((classname**)ud); \
}


CREATEBIND(Map);
CREATEBIND(TileLayer);
CREATEBIND(Tileset);
CREATEBIND(Tile);

static int luatiled_mapIndex(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 2) {
        Map* map = checkMap(L, 1);
        if (lua_isnumber(L, 2)) {
            int idx = luaL_checknumber(L, 2);
            if(idx >= map->layerCount())
                luaL_error(L, "Out of bound: %d is bigger than %d", idx, map->layerCount());
            else
                luatiled_createTileLayer(L, (TileLayer*)map->layerAt(idx));
        } else {
            const char * key = luaL_checkstring(L, 2);
            if(strcmp(key, "name")==0) {
                lua_pushstring(L, "Tiled Map");
            } else {
                int idx = map->indexOfLayer(QObject::tr(key));
                if(idx > -1)
                    luatiled_createTileLayer(L, (TileLayer*)map->layerAt(idx));
                else {
                    lua_getmetatable(L, 1);
                    lua_pushvalue(L, 2);
                    lua_rawget(L, -2);
                }
            }
        }
    }
    else
        luaL_error(L, "Got %d arguments expected 2 (self,number)", n);

    return 1;
}

static int luatiled_getTileset(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 2) {
        Map* map = checkMap(L, 1);
        if (lua_isnumber(L, 2)) {
            int idx = luaL_checknumber(L, 2);
            if(idx >= map->tilesets().count())
                luaL_error(L, "Out of bound: %d is bigger than %d",
                           idx, map->tilesets().count());
            else
                luatiled_createTileset(L, (Tileset*)map->tilesets().at(idx));
        } else {
            const char *tilesetName = luaL_checkstring(L, 2);

            Tileset *tileset = map->findTileset(QObject::tr(tilesetName));
            if (tileset)
                luatiled_createTileset(L, tileset);
            else {
                lua_getmetatable(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
            }
        }
    }
    else
        luaL_error(L, "Got %d arguments expected 2 (map, string)", n);

    return 1;
}

static int luatiled_tilesetTileAt(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 3) {
        Tileset *tileset = checkTileset(L, 1);
        if (lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
            int x = luaL_checknumber(L, 2);
            int y = luaL_checknumber(L, 3);
            int idx = (y*tileset->columnCount()) + x;
            if(idx >= tileset->tileCount())
                luaL_error(L, "Out of bound: %d is bigger than %d",
                           idx, tileset->tileCount());
            else
                luatiled_createTile(L, (Tile*)tileset->tileAt(idx));
        } else {
            luaL_error(L, "Expected (self, number, number)");
        }
    }
    else
        luaL_error(L, "Got %d arguments expected 3 (self,number,number)", n);

    return 1;
}

static int luatiled_tilesetIndex(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 2) {
        Tileset *tileset = checkTileset(L, 1);
        if (lua_isnumber(L, 2)) {
            int idx = luaL_checknumber(L, 2);
            if(idx >= tileset->tileCount())
                luaL_error(L, "Out of bound: %d is bigger than %d",
                           idx, tileset->tileCount());
            else
                luatiled_createTile(L, (Tile*)tileset->tileAt(idx));
        } else {
            const char * key = luaL_checkstring(L, 2);
            if (strcmp(key, "columnCount")==0)
                lua_pushnumber(L, tileset->columnCount());
            else if (strcmp(key, "tileAt")==0)
                lua_pushcfunction(L, luatiled_tilesetTileAt);
            else {
                lua_getmetatable(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
            }
        }
    }
    else
        luaL_error(L, "Got %d arguments expected 2 (self,number)", n);

    return 1;
}

static int luatiled_tileLayerIndex(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 2) {
        TileLayer* tilelayer = checkTileLayer(L, 1);
        if (lua_isnumber(L, 2)) {
            int y = luaL_checknumber(L, 2);
            lua_pushstring(L, "__arg1");
            lua_rawget(L, 1);
            if (lua_isnil(L, -1)) {
                lua_pushstring(L, "__arg1");
                lua_pushvalue(L, 2);
                lua_rawset(L, 1);
                lua_settop(L, 1);
            } else {
                int x = luaL_checknumber(L, -1);
                lua_pushstring(L, "__arg1");
                lua_pushnil(L);
                lua_rawset(L, 1);
                if (x > tilelayer->width()) {
                    luaL_error(L, "Out of bound: %d is bigger than %d",
                               x, tilelayer->width());
                    return 1;
                }
                if (y > tilelayer->height()) {
                    luaL_error(L, "Out of bound: %d is bigger than %d",
                               y, tilelayer->height());
                    return 1;
                }

                luatiled_createTile(L, tilelayer->cellAt(x, y).tile);
            }
        } else {
            const char * key = luaL_checkstring(L, 2);
            if (strcmp(key, "width")==0)
                lua_pushnumber(L, tilelayer->width());
            else if (strcmp(key, "height")==0)
                lua_pushnumber(L, tilelayer->height());
            else {
                lua_getmetatable(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
            }
        }
    }
    else
        luaL_error(L, "Got %d arguments expected 2 (self,number)", n);

    return 1;
}

static int luatiled_setTile(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 3) {
        TileLayer* tilelayer = checkTileLayer(L, 1);
        if (lua_isnumber(L, 2)) {
            int y = luaL_checknumber(L, 2);
            lua_pushstring(L, "__arg1");
            lua_rawget(L, 1);
            if (!lua_isnil(L, -1)) {
                int x = luaL_checknumber(L, -1);
                lua_pushstring(L, "__arg1");
                lua_pushnil(L);
                lua_rawset(L, 1);
                Tile* tile = NULL;
                if (!lua_isnil(L, 3))
                    tile = checkTile(L, 3);

                if (x > tilelayer->width()) {
                    luaL_error(L, "Out of bound: %d is bigger than %d",
                               x, tilelayer->width());
                    return 1;
                }
                if (y > tilelayer->height()) {
                    luaL_error(L, "Out of bound: %d is bigger than %d",
                               y, tilelayer->height());
                    return 1;
                }
                lua_pushstring(L, "__mapdocument__");
                lua_gettable(L, LUA_REGISTRYINDEX);
                MapDocument* mapDocument = reinterpret_cast<MapDocument*>(lua_touserdata(L, -1));
                TilePainter tilePainter(mapDocument, tilelayer);
                tilePainter.setCell(x, y, Cell(tile));
            }
        }
        else
            luaL_error(L, "Wrong format: expected layer[x][y]=tile");
    }
    else
        luaL_error(L, "Got %d arguments expected 3 (self,number,Tile)", n);

    return 1;
}

static int luatiled_getTileId(lua_State *L)
{
    int n = lua_gettop(L);  // Number of arguments

    if (n == 1) {
        Tile* tile = checkTile(L, 1);
        lua_pushinteger(L, tile->id());
    }
    else
        luaL_error(L, "Got %d arguments expected 1 (self)", n);

    return 1;
}

static void registerLuaBinds(lua_State *L, Map *activeMap)
{
    //Metatables for userdata
    luaL_newmetatable(L, "Tiled.Map");
    luaL_newmetatable(L, "Tiled.TileLayer");
    luaL_newmetatable(L, "Tiled.Tileset");
    luaL_newmetatable(L, "Tiled.Tile");
    lua_pop(L, 4);

    luaL_Reg map_funcs[] = {
        {"__index", luatiled_mapIndex},
        {"getTileset", luatiled_getTileset},
        {NULL, NULL}
    };
    // Map as a reference to the activeMap
    // and is metatable of itself to resolve layers
    luatiled_createMap(L, activeMap);
    luaL_register(L, NULL, map_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "Map");
    lua_setmetatable(L, -1);

    luaL_Reg tilelayer_funcs[] = {
        {"__index", luatiled_tileLayerIndex},
        {"__newindex", luatiled_setTile},
        {NULL, NULL}
    };
    luaL_register(L, "TileLayer", tilelayer_funcs);
    lua_pop(L, 1);

    luaL_Reg tileset_funcs[] = {
        {"__index", luatiled_tilesetIndex},
        {NULL, NULL}
    };
    luaL_register(L, "Tileset", tileset_funcs);
    lua_pop(L, 1);

    luaL_Reg tile_funcs[] = {
        {"getId", luatiled_getTileId},
        {NULL, NULL}
    };
    luaL_register(L, "Tile", tile_funcs);
    lua_pop(L, 1);
}

#endif // LUABINDS_H
