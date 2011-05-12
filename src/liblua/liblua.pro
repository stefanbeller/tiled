include(../../tiled.pri)

TEMPLATE = lib
TARGET = lua
target.path = $${LIBDIR}
INSTALLS += target
macx {
    DEFINES += LUA_USE_MACOSX
    DESTDIR = ../../bin/Tiled.app/Contents/Frameworks
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
} else {
    linux-g++ {
        DEFINES += LUA_USE_LINUX
    }
    DESTDIR = ../../lib
}
DLLDESTDIR = ../..

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
DEFINES += LUA_LIBRARY
#contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols
OBJECTS_DIR = .obj
SOURCES += lapi.c \
    lauxlib.c \
    lbaselib.c \
    lcode.c \
    ldblib.c \
    ldebug.c \
    ldo.c \
    ldump.c \
    lfunc.c \
    lgc.c \
    linit.c \
    liolib.c \
    llex.c \
    lmathlib.c \
    lmem.c \
    loadlib.c \
    lobject.c \
    lopcodes.c \
    loslib.c \
    lparser.c \
    lstate.c \
    lstring.c \
    lstrlib.c \
    ltable.c \
    ltablib.c \
    ltm.c \
    lundump.c \
    lvm.c \
    lzio.c \
    print.c

HEADERS += lapi.h \
    lauxlib.h \
    lcode.h \
    ldebug.h \
    ldo.h \
    lfunc.h \
    lgc.h \
    llex.h \
    llimits.h \
    lmem.h \
    lobject.h \
    lopcodes.h \
    lparser.h \
    lstate.h \
    lstring.h \
    ltable.h \
    ltm.h \
    lua.h \
    luaconf.h \
    lualib.h \
    lundump.h \
    lvm.h \
    lzio.h

macx {
    contains(QT_CONFIG, ppc):CONFIG += x86 \
        ppc
}
