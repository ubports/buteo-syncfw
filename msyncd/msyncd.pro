TEMPLATE = app
TARGET = 
QT += xml \
    dbus \
    sql \
    network
QT -= gui
CONFIG += qdbus \
    debug \
    link_pkgconfig \
    create_pc \
    create_prl \
    mobility

MOBILITY += systeminfo

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc

DEPENDPATH += . 
INCLUDEPATH += . \
    ../ \
    ../libsynccommon \
    ../libsyncpluginmgr \
    ../libsyncprofile \
    /usr/include/accounts-qt \
    /usr/include/iphbd/ \
    
PKGCONFIG += dbus-1
QMAKE_LIBDIR_QT += ../libsyncprofile/

LIBS += -L../libsyncpluginmgr \
    -L../libsyncprofile \
    -L../libsynccommon \
    -lsyncpluginmgr \
    -lsyncprofile \
    -lsynccommon \
    -laccounts-qt \
# libqttracker has a bug (158705) that causes a crash when tracker is used from
# a library (storage plug-in in our case), which is then unloaded after use. To
# prevent that crash we need to link with libqttracker here also, so that the
# library will not be unloaded when storage plug-in is unloaded. The bug has been
# marked as WONTFIX. However, as another workaround for NB #160965, we do not
# unload libraries anymore, so the linkage to libqttracker can be removed now.
#   -lqttracker \
    -liphb 


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
    SyncSigHandler.h

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
    SyncSigHandler.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 \
    -finline-functions

# install
target.path = /usr/bin/
loglevel.files = bin/set_sync_log_level
loglevel.path = /etc/sync/
meego.files = bin/msyncd.desktop
meego.path = /etc/xdg/autostart/
INSTALLS += target \
    loglevel

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
