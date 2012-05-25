HEADERS += \
    client.h
SOURCES += \
    client.cpp \
    main.cpp

win32 {
        SOURCES += qt/mfilesystemwatcher_win.cpp \
            qt/mfilesystemwatcher.cpp         
        HEADERS += qt/mfilesystemwatcher_win_p.h \
            qt/mfilesystemwatcher_p.h \
            qt/mfilesystemwatcher.h
        INCLUDEPATH += $$(QT_SOURCE)\include
}
FORMS += \
    client.ui
QT += network svg xml
RESOURCES += \
    systray.qrc

REVISION = $$system(git rev-parse --short HEAD)
VERSION = 1.1.1
VERSTR = '\\"$${VERSION}\\"'
REVSTR = '\\"$${REVISION}\\"'
DEFINES += VER=\"$${VERSTR}\" REV=\"$${REVSTR}\"

DESTDIR = ./build
OBJECTS_DIR = ./build
TARGET = easysync-client

CONFIG(release, release|debug) {
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    DEFINES -= QT_NO_DEBUG_OUTPUT
}
