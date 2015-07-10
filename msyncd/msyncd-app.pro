TEMPLATE = app
TARGET = msyncd
QT += xml \
    dbus \
    sql \
    network
QT -= gui

CONFIG += \
    link_pkgconfig \
    link_prl

DEPENDPATH += .
INCLUDEPATH += . \
    ../ \
    ../libbuteosyncfw/pluginmgr \
    ../libbuteosyncfw/common \
    ../libbuteosyncfw/profile


PKGCONFIG += dbus-1

equals(QT_MAJOR_VERSION, 4): {
    PKGCONFIG += libsignon-qt accounts-qt
    CONFIG += mobility
    MOBILITY += systeminfo
    LIBS += -lbuteosyncfw
}
equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG += libsignon-qt5 accounts-qt5 Qt5SystemInfo
    LIBS += -lbuteosyncfw5
    packagesExist(qt5-boostable) {
        DEFINES += HAS_BOOSTER
        PKGCONFIG += qt5-boostable
    } else {
        warning("qt5-boostable not available; startup times will be slower")
    }
}

QMAKE_LIBDIR_QT += ../libsyncprofile/

LIBS += -L../libbuteosyncfw

LIBS += -L. -lmsyncd

# Input
SOURCES += \
        main.cpp \
        UnitTest.cpp

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
gschemas.path = /usr/share/glib-2.0/schemas
gschemas.files = gschemas/com.meego.msyncd.gschema.xml
INSTALLS += target \
    loglevel \
    syncwidget \
    service \
    gschemas

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
