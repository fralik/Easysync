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
}
FORMS += \
    client.ui
QT += network svg xml
RESOURCES += \
    systray.qrc

DESTDIR = ./build
OBJECTS_DIR = ./build
TARGET = easysync-client

CONFIG(release, release|debug) {
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    DEFINES -= QT_NO_DEBUG_OUTPUT
}
