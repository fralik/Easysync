QT       += core network sql
QT       -= gui

TARGET = easysync-server
CONFIG   += console qt
CONFIG   -= app_bundle
DESTDIR = ./build
OBJECTS_DIR = ./build
INCLUDEPATH += ./qt

TEMPLATE = app

REVISION = $$system(git rev-parse --short HEAD)
VERSION = 1.1.2
VERSTR = '\\"$${VERSION}\\"'
REVSTR = '\\"$${REVISION}\\"'
DEFINES += VER=\"$${VERSTR}\" REV=\"$${REVSTR}\" NEED_PRINTF=1

SOURCES += main.cpp \
    easysyncserver.cpp \
    easysyncservice.cpp \
    dbmanager.cpp

HEADERS += \
    easysyncserver.h \
    easysyncservice.h \
    qtservice.h \
    dbmanager.h
include(./qt/qtservice.pri)

FORMS +=

CONFIG(release, release|debug) {
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    DEFINES -= QT_NO_DEBUG_OUTPUT
}
