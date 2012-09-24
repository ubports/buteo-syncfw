TEMPLATE = app
TARGET = sync-fw-tests
DEPENDPATH += . \
    pluginmanagertests \
    syncprofiletests \
    msyncdtests \
    syncfwclienttests \
    ../../msyncd

INCLUDEPATH += . \
    ../.. \
    ../../libsynccommon \
    ../../libsyncpluginmgr \
    ../../libsyncprofile \
    ../../msyncd \
    ../../libsyncfwclient \
    pluginmanagertests \
    syncprofiletests \
    msyncdtests \
    syncfwclienttests \
    /usr/include/accounts-qt

DEFINES += SYNCFW_UNIT_TESTS 

SOURCES += tests.cpp \
    SyncFwTestRunner.cpp \

HEADERS += SyncFwTestRunner.h \
    SyncFwTestLoader.h

include(pluginmanagertests/pluginmanagertests.pro)
include(syncprofiletests/syncprofiletests.pro)
include(msyncdtests/msyncdtests.pro)
include(syncfwclienttests/syncfwclienttests.pro)

CONFIG += qtestlib \
    link_prl \
    qdbus

# This is needed to avoid adding the /usr/lib link directory before the
# newer version in the present directories
QMAKE_LIBDIR_QT = ../../libsynccommon \
    ../../libsyncprofile \
    ../../libsyncpluginmgr \
    ../../libsyncfwclient 
    
LIBS += -lsynccommon \
    -lsyncprofile \
    -lsyncpluginmgr \
    -lsyncfwclient \
    -ldbus-1 \
    -lgcov \
    -laccounts-qt

# install
tests.files = \
	      runstarget.sh \
	      sync-fw-tests.ref

testdefinition.files = tests.xml

testprofiles_client.files = syncprofiletests/testprofiles/user/client/*
testprofiles_client.path = /opt/tests/buteo-syncfw/syncprofiletests/testprofiles/user/client/

testprofiles_service.files = syncprofiletests/testprofiles/user/service/*
testprofiles_service.path = /opt/tests/buteo-syncfw/syncprofiletests/testprofiles/user/service/

testprofiles_storage.files = syncprofiletests/testprofiles/user/storage/*
testprofiles_storage.path = /opt/tests/buteo-syncfw/syncprofiletests/testprofiles/user/storage/

testprofiles_sync.files = syncprofiletests/testprofiles/user/sync/*
testprofiles_sync.path = /opt/tests/buteo-syncfw/syncprofiletests/testprofiles/user/sync/

testprofiles_syssync.files = syncprofiletests/testprofiles/system/sync/*
testprofiles_syssync.path = /opt/tests/buteo-syncfw/syncprofiletests/testprofiles/system/sync/

testaccount_service.files = syncprofiletests/testprofiles/testsync-ovi.service
testaccount_service.path = /usr/share/accounts/services/

	      
tests.path = /opt/tests/buteo-syncfw/
target.path = /opt/tests/buteo-syncfw/
testdefinition.path = /opt/tests/buteo-syncfw/test-definition/

INSTALLS += tests \
    target \
    testaccount_service \
    testprofiles_client \
    testprofiles_service \
    testprofiles_storage \
    testprofiles_sync \
    testprofiles_syssync \
    testdefinition

QT += testlib \
    core \
    xml \
    dbus \
    network \
    sql
QT -= gui
QMAKE_CXXFLAGS += -Wall \
        -fprofile-arcs \
        -ftest-coverage \
         -g -O0

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
