TEMPLATE = app
TARGET = buteo-oopp-runner

QT += dbus
QT -= gui

INCLUDEPATH += $$PWD \
    ../libbuteosyncfw/pluginmgr \
    ../libbuteosyncfw/clientfw \
    ../libbuteosyncfw/common \
    ../libbuteosyncfw/profile

LIBS += -lbuteosyncfw5
LIBS += -L../libbuteosyncfw

HEADERS += ButeoPluginIfaceAdaptor.h \
           PluginCbImpl.h \
           PluginServiceObj.h

SOURCES += ButeoPluginIfaceAdaptor.cpp \
           PluginCbImpl.cpp \
           PluginServiceObj.cpp \
           main.cpp

target.path = /usr/libexec/
INSTALLS += target
