TEMPLATE = lib

VER_MAJ = 0
VER_MIN = 4
VER_PAT = 3

TARGET = syncprofile
DEPENDPATH += .
INCLUDEPATH += . \
    ../libsynccommon

QT += xml dbus
QT -= gui
CONFIG += dll \
    create_pc \
    create_prl

# Input
HEADERS += Profile.h \
    ProfileFactory.h \
    TargetResults.h \
    SyncResults.h \
    SyncProfile.h \
    SyncLog.h \
    ProfileManager.h \
    ProfileField.h \
    ProfileEngineDefs.h \
    StorageProfile.h \
    SyncSchedule.h \
    Profile_p.h \
    SyncSchedule_p.h \
    BtHelper.h

SOURCES += Profile.cpp \
    ProfileManager.cpp \
    ProfileFactory.cpp \
    ProfileField.cpp \
    TargetResults.cpp \
    SyncResults.cpp \
    SyncProfile.cpp \
    SyncLog.cpp \
    StorageProfile.cpp \
    SyncSchedule.cpp \
    BtHelper.cpp

QMAKE_CXXFLAGS += -Wall \
    -g \
    -Wno-cast-align \
    -O2 \
    -finline-functions


# clean
QMAKE_CLEAN += $(TARGET) \
    $(TARGET0) \
    $(TARGET1) \
    $(TARGET2) \
    $(OBJECTS_DIR)/moc_* \
    lib$${TARGET}.prl pkgconfig/*

# install
target.path = /usr/lib/
headers.path = /usr/include/libsyncprofile
headers.files = Profile.h \
    ProfileFactory.h \
    TargetResults.h \
    SyncResults.h \
    SyncProfile.h \
    SyncLog.h \
    SyncSchedule.h \
    ProfileManager.h \
    ProfileField.h \
    BtHelper.h \
    ProfileEngineDefs.h
utility.path = /opt/tests/buteo-syncfw
utility.files = bin/*.pl \
    bin/*.sh
INSTALLS += target \
    headers \
    utility

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc

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
    coverage.commands = @echo \
        "Built with coverage support..."
    build_pass|!debug_and_release:coverage.depends = all
    QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda \
        $(OBJECTS_DIR)/*.gcno \
        $(OBJECTS_DIR)/*.gcov
}
OTHER_FILES += 
