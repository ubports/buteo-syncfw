TEMPLATE = lib

VER_MAJ = 0
VER_MIN = 4
VER_PAT = 4

TARGET = synccommon
DEPENDPATH += .
INCLUDEPATH += .
QT -= gui
CONFIG += dll \
    debug \
    silent \
    create_pc \
    create_prl

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc

# Input
SOURCES += Logger.cpp
HEADERS += Logger.h \
    LogMacros.h \
    SyncCommonDefs.h
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
headers.path = /usr/include/libsynccommon
headers.files = LogMacros.h \
    Logger.h \
    SyncCommonDefs.h
INSTALLS += target \
    headers

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
