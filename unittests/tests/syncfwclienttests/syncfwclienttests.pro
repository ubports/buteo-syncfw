SOURCEPATH += . \
    ../../../libsyncfwclient

INCLUDEPATH += ../ \
                        ../../msyncd


QT += xml \
    dbus
QT -= gui
CONFIG += \
    link_pkgconfig \

PKGCONFIG += dbus-1
LIBS +=

SOURCES +=  SyncClientInterfaceTest.cpp

HEADERS +=  SyncClientInterfaceTest.h

OTHER_FILES +=
