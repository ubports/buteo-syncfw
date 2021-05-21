isEmpty(TESTS_COMMON_PRI_INCLUDED) {
TESTS_COMMON_PRI_INCLUDED = 1

tests_subdir = $$relative_path($$dirname(_PRO_FILE_), $${PWD})
tests_subdir_r = $$relative_path($${PWD}, $$dirname(_PRO_FILE_))

QT += testlib \
    core \
    dbus \
    xml \
    dbus \
    network \
    sql

QT -= gui

CONFIG += link_pkgconfig link_prl

PKGCONFIG += dbus-1

LIBS += -lgcov

LIBS += -L$${OUT_PWD}/$${tests_subdir_r}/../../libbuteosyncfw

PKGCONFIG += libsignon-qt5 accounts-qt5
LIBS += -lbuteosyncfw5

# This is needed to avoid adding the /usr/lib link directory before the
# newer version in the present directories
QMAKE_LIBDIR_QT = $${OUT_PWD}/$${tests_subdir_r}/../../libbuteosyncfw

DEFINES += SYNCFW_UNIT_TESTS

QMAKE_CXXFLAGS += -Wall \
        -fprofile-arcs \
        -ftest-coverage \
         -g -O0

INCLUDEPATH = \
    $${PWD} \
    $${PWD}/../.. \
    $${PWD}/../../libbuteosyncfw/clientfw \
    $${PWD}/../../libbuteosyncfw/common \
    $${PWD}/../../libbuteosyncfw/pluginmgr \
    $${PWD}/../../libbuteosyncfw/profile \
    $${PWD}/../../msyncd \

CONFIG -= depend_includepath
DEPENDPATH = \
    $${PWD}/../../libbuteosyncfw/clientfw \
    $${PWD}/../../libbuteosyncfw/common \
    $${PWD}/../../libbuteosyncfw/pluginmgr \
    $${PWD}/../../libbuteosyncfw/profile \
    $${PWD}/../../msyncd \

INSTALL_TESTDIR = /opt/tests/buteo-syncfw
INSTALL_TESTDATADIR = $${INSTALL_TESTDIR}/data

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

}
