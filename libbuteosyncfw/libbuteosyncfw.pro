TEMPLATE = lib
equals(QT_MAJOR_VERSION, 4): TARGET = buteosyncfw
equals(QT_MAJOR_VERSION, 5): TARGET = buteosyncfw5
DEPENDPATH += . clientfw  common  pluginmgr  profile
INCLUDEPATH += . clientfw  common  pluginmgr  profile

VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0

QT += sql xml dbus

QT -= gui

CONFIG += dll \
    create_pc \
    create_prl

# Input
HEADERS += common/Logger.h \
           common/LogMacros.h \
           common/SyncCommonDefs.h \
           common/SyncDBusConnection.h \
           clientfw/SyncClientInterface.h \
           clientfw/SyncClientInterfacePrivate.h \
           clientfw/SyncDaemonProxy.h \
           pluginmgr/ClientPlugin.h \
           pluginmgr/DeletedItemsIdStorage.h \
           pluginmgr/PluginCbInterface.h \
           pluginmgr/PluginManager.h \
           pluginmgr/ServerPlugin.h \
           pluginmgr/StorageChangeNotifierPlugin.h \
           pluginmgr/StorageItem.h \
           pluginmgr/StoragePlugin.h \
           pluginmgr/SyncPluginBase.h \
           profile/BtHelper.h \
           profile/Profile.h \
           profile/Profile_p.h \
           profile/ProfileEngineDefs.h \
           profile/ProfileFactory.h \
           profile/ProfileField.h \
           profile/ProfileManager.h \
           profile/StorageProfile.h \
           profile/SyncLog.h \
           profile/SyncProfile.h \
           profile/SyncResults.h \
           profile/SyncSchedule.h \
           profile/SyncSchedule_p.h \
           profile/TargetResults.h
SOURCES += common/Logger.cpp \
           common/SyncDBusConnection.cpp \
           clientfw/SyncClientInterface.cpp \
           clientfw/SyncClientInterfacePrivate.cpp \
           clientfw/SyncDaemonProxy.cpp \
           pluginmgr/ClientPlugin.cpp \
           pluginmgr/DeletedItemsIdStorage.cpp \
           pluginmgr/PluginManager.cpp \
           pluginmgr/ServerPlugin.cpp \
           pluginmgr/StorageItem.cpp \
           pluginmgr/StoragePlugin.cpp \
           pluginmgr/SyncPluginBase.cpp \
           profile/BtHelper.cpp \
           profile/Profile.cpp \
           profile/ProfileFactory.cpp \
           profile/ProfileField.cpp \
           profile/ProfileManager.cpp \
           profile/StorageProfile.cpp \
           profile/SyncLog.cpp \
           profile/SyncProfile.cpp \
           profile/SyncResults.cpp \
           profile/SyncSchedule.cpp \
           profile/TargetResults.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

# clean
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)

QMAKE_CLEAN += $(OBJECTS_DIR)/moc_*
QMAKE_CLEAN += lib$${TARGET}.prl pkgconfig/*

# install
target.path = /usr/lib/

equals(QT_MAJOR_VERSION, 4): headers.path = /usr/include/buteosyncfw
equals(QT_MAJOR_VERSION, 5): headers.path = /usr/include/buteosyncfw5

headers.files = common/Logger.h \
           common/LogMacros.h \
           common/SyncCommonDefs.h \
           common/SyncDBusConnection.h \
           clientfw/SyncClientInterface.h \
           clientfw/SyncClientInterfacePrivate.h \
           clientfw/SyncDaemonProxy.h \
           pluginmgr/ClientPlugin.h \
           pluginmgr/DeletedItemsIdStorage.h \
           pluginmgr/PluginCbInterface.h \
           pluginmgr/PluginManager.h \
           pluginmgr/ServerPlugin.h \
           pluginmgr/StorageChangeNotifierPlugin.h \
           pluginmgr/StorageItem.h \
           pluginmgr/StoragePlugin.h \
           pluginmgr/SyncPluginBase.h \
           profile/BtHelper.h \
           profile/Profile.h \
           profile/Profile_p.h \
           profile/ProfileEngineDefs.h \
           profile/ProfileFactory.h \
           profile/ProfileField.h \
           profile/ProfileManager.h \
           profile/StorageProfile.h \
           profile/SyncLog.h \
           profile/SyncProfile.h \
           profile/SyncResults.h \
           profile/SyncSchedule.h \
           profile/SyncSchedule_p.h \
           profile/TargetResults.h

utility.path = /opt/tests/buteo-syncfw
utility.files = ../bin/*.pl \
    ../bin/*.sh

INSTALLS += target headers utility

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc

# #####################################################################
# make coverage (debug)
# #####################################################################
coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
CONFIG(debug,debug|release){
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

    coverage.commands = @echo \
        "Built with coverage support..."
    build_pass|!debug_and_release : coverage.depends = all
    QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda \
        $(OBJECTS_DIR)/*.gcno \
        $(OBJECTS_DIR)/*.gcov
}
