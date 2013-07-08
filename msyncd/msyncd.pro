TEMPLATE = app
TARGET = msyncd
QT += xml \
    dbus \
    sql \
    network
QT -= gui

CONFIG += \
    link_pkgconfig \
    create_pc \
    create_prl

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc

DEPENDPATH += .
INCLUDEPATH += . \
    ../ \
    ../libbuteosyncfw/pluginmgr \
    ../libbuteosyncfw/common \
    ../libbuteosyncfw/profile


PKGCONFIG += dbus-1 libiphb

equals(QT_MAJOR_VERSION, 4): {
    PKGCONFIG += libsignon-qt accounts-qt
    CONFIG += mobility
    MOBILITY += systeminfo
    LIBS += -lbuteosyncfw
}
equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG += libsignon-qt5 accounts-qt5 Qt5SystemInfo
    LIBS += -lbuteosyncfw5
}

QMAKE_LIBDIR_QT += ../libsyncprofile/

LIBS += -L../libbuteosyncfw

# Input
HEADERS += ServerActivator.h \
    TransportTracker.h \
    synchronizer.h \
    SyncDBusInterface.h \
    SyncBackupProxy.h \
    SyncDBusAdaptor.h \
    SyncBackupAdaptor.h \
    ClientThread.h \
    ServerThread.h \
    StorageBooker.h \
    SyncQueue.h \
    SyncScheduler.h \
    SyncBackup.h \
    AccountsHelper.h \
    SyncSession.h \
    PluginRunner.h \
    ClientPluginRunner.h \
    ServerPluginRunner.h \
    SyncAlarmInventory.h \
    NetworkManager.h \
    IPHeartBeat.h \
    SyncSigHandler.h \
    StorageChangeNotifier.h \
    SyncOnChange.h \
    SyncOnChangeScheduler.h

SOURCES += ServerActivator.cpp \
    TransportTracker.cpp \
    main.cpp \
    synchronizer.cpp \
    SyncDBusAdaptor.cpp \
    SyncBackupAdaptor.cpp \
    ClientThread.cpp \
    ServerThread.cpp \
    StorageBooker.cpp \
    SyncQueue.cpp \
    SyncScheduler.cpp \
    SyncBackup.cpp \
    AccountsHelper.cpp \
    SyncSession.cpp \
    PluginRunner.cpp \
    ClientPluginRunner.cpp \
    ServerPluginRunner.cpp \
    SyncAlarmInventory.cpp \
    NetworkManager.cpp \
    IPHeartBeat.cpp \
    SyncSigHandler.cpp \
    StorageChangeNotifier.cpp \
    SyncOnChange.cpp \
    SyncOnChangeScheduler.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 \
    -finline-functions

# install
target.path = /usr/bin/
loglevel.files = bin/set_sync_log_level
loglevel.path = /etc/buteo/
service.files = bin/msyncd.service
service.path = /usr/lib/systemd/user/
syncwidget.path = /etc/syncwidget/
syncwidget.files = com.meego.msyncd
INSTALLS += target \
    loglevel \
    syncwidget \
    service

# clean
QMAKE_CLEAN += $(TARGET)
QMAKE_CLEAN += $(OBJECTS_DIR)/moc_*
QMAKE_CLEAN += lib$${TARGET}.prl pkgconfig/*

# #####################################################################
# make coverage (debug)
# #####################################################################
coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
CONFIG(debug,debug|release) {
    QMAKE_EXTRA_TARGETS += cov_cxxflags \
        cov_lflags
    cov_cxxflags.target = coverage
    cov_cxxflags.depends = CXXFLAGS \
        += \
        -fprofile-arcs \
        -ftest-coverage
    cov_lflags.target = coverage
    cov_lflags.depends = LFLAGS \
        += \
        -fprofile-arcs \
        -ftest-coverage

    # QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    # QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
    # -ftest-coverage
    coverage.commands = @echo \
        "Built with coverage support..."
    build_pass|!debug_and_release:coverage.depends = all
    QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda \
        $(OBJECTS_DIR)/*.gcno \
        $(OBJECTS_DIR)/*.gcov
}

# for compiling on meego
linux-g++-maemo {
  message("Maemo specific install")
  DEFINES += __USBMODED__
  HEADERS += USBModedProxy.h
  SOURCES += USBModedProxy.cpp
} else {
  message("Meego specific install")
  INSTALLS += meego
}
