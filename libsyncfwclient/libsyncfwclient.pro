TEMPLATE = lib
VER_MAJ = 0
VER_MIN = 5
VER_PAT = 0
TARGET = syncfwclient
DEPENDPATH += .
INCLUDEPATH += ../ \
    ../libsynccommon/
QT += xml \
    dbus
QT -= gui
CONFIG += dll \
    debug \
    silent \
    create_pc \
    create_prl

LIBS += -L../libsyncprofile/ \
        -lsyncprofile 

# Input
HEADERS += SyncClientInterface.h \
    SyncClientInterfacePrivate.h \
    SyncDaemonProxy.h 
SOURCES += SyncClientInterfacePrivate.cpp \
    SyncClientInterface.cpp \
    SyncDaemonProxy.cpp

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
headers.path = /usr/include/sync/
headers.files = SyncClientInterface.h
INSTALLS += target \
    headers

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
