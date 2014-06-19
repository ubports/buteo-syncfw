include(../../../msyncd/unittest.pri)

QT += xml \
    dbus
QT -= gui
CONFIG += \
    link_pkgconfig

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    MOBILITY += systeminfo
}

PKGCONFIG += dbus-1

equals(QT_MAJOR_VERSION, 5): PKGCONFIG += Qt5SystemInfo

LIBS += -L../../../libbuteosyncfw -lbuteosyncfw5

SOURCES += \
    ServerThreadTest.cpp \
    StorageBookerTest.cpp \
    SyncQueueTest.cpp \
    AccountsHelperTest.cpp \
    SyncSessionTest.cpp \
    ClientThreadTest.cpp\
    TransportTrackerTest.cpp \
    PluginRunnerTest.cpp \
    ServerActivatorTest.cpp \
    ClientPluginRunnerTest.cpp \
    SyncBackupTest.cpp \
    ServerPluginRunnerTest.cpp \
    SynchronizerTest.cpp \
    SyncSigHandlerTest.cpp \

HEADERS += \
    ServerThreadTest.h \
    StorageBookerTest.h \
    SyncQueueTest.h \
    AccountsHelperTest.h \
    SyncSessionTest.h \
    ClientThreadTest.h\
    TransportTrackerTest.h \
    PluginRunnerTest.h \
    ServerActivatorTest.h \
    ClientPluginRunnerTest.h \
    SyncBackupTest.h \
    ServerPluginRunnerTest.h \
    SynchronizerTest.h \
    SyncSigHandlerTest.h \
