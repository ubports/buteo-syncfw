SOURCEPATH += . \
    ../../../msyncd

DEFINES +=  UNIT_TEST
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

SOURCES += ServerThread.cpp \
    ServerThreadTest.cpp \
    StorageBookerTest.cpp \
    StorageBooker.cpp \
    SyncQueueTest.cpp \
    SyncQueue.cpp \
    AccountsHelperTest.cpp \
    AccountsHelper.cpp \
    SyncScheduler.cpp \
    SyncSession.cpp \
    SyncSessionTest.cpp \
    PluginRunner.cpp \
    ClientThread.cpp \
    ClientThreadTest.cpp\
    TransportTrackerTest.cpp \
    PluginRunnerTest.cpp \
    ClientPluginRunner.cpp \
    ServerActivator.cpp \
    ServerActivatorTest.cpp \
    ClientPluginRunnerTest.cpp \
    synchronizer.cpp \
    SyncDBusAdaptor.cpp \
    SyncBackupAdaptor.cpp \
    SyncBackup.cpp \
    SyncBackupTest.cpp \
    ServerPluginRunner.cpp \
    ServerPluginRunnerTest.cpp \
    SynchronizerTest.cpp \
    SyncAlarmInventory.cpp \
    NetworkManager.cpp \
    SyncSigHandler.cpp \
    SyncSigHandlerTest.cpp \
    SyncOnChange.cpp \
    SyncOnChangeScheduler.cpp \
    StorageChangeNotifier.cpp

HEADERS += ServerThread.h \
    ServerThreadTest.h \
    StorageBookerTest.h \
    SyncQueueTest.h \
    AccountsHelperTest.h \
    AccountsHelper.h \
    SyncScheduler.h \
    SyncSession.h \
    SyncSessionTest.h \
    PluginRunner.h \
    ClientThread.h \
    ClientThreadTest.h\
    TransportTrackerTest.h \
    PluginRunnerTest.h \
    ClientPluginRunner.h \
    ServerActivator.h \
    ServerActivatorTest.h \
    ClientPluginRunnerTest.h \
    synchronizer.h \
    SyncDBusAdaptor.h \
    SyncDBusInterface.h \
    SyncBackupProxy.h \
    SyncBackupAdaptor.h \
    SyncBackup.h \
    SyncBackupTest.h \
    ServerPluginRunner.h \
    ServerPluginRunnerTest.h \
    SyncAlarmInventory.h \
    SynchronizerTest.h \
    NetworkManager.h \
    SyncSigHandler.h \
    SyncSigHandlerTest.h \
    SyncOnChange.h \
    SyncOnChangeScheduler.h \
    StorageChangeNotifier.h

contains(DEFINES, USE_KEEPALIVE) {
    PKGCONFIG += keepalive

    HEADERS += \
        BackgroundSync.h

    SOURCES += \
        BackgroundSync.cpp

} else {
    PKGCONFIG += libiphb

    HEADERS += \
        SyncSchedulerTest.h \
        IPHeartBeat.h \
        IPHeartBeatTest.h

    SOURCES += \
        SyncSchedulerTest.cpp \
        IPHeartBeat.cpp \
        IPHeartBeatTest.cpp
}

# for compiling on meego
linux-g++-maemo {
  message("Compiling with USBModed support")
  DEFINES += __USBMODED__
  HEADERS += USBModedProxy.h
  SOURCES += USBModedProxy.cpp
} else {
  message("Compiling without USBModed")
}
