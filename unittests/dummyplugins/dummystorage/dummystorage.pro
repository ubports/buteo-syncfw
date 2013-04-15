TEMPLATE = lib
TARGET = hdummy-storage
DEPENDPATH += .
INCLUDEPATH += . \
    ../../.. \
    ../../../libbuteosyncfw/common \
    ../../../libbuteosyncfw/pluginmgr \
    ../../../libbuteosyncfw/profile

CONFIG += plugin
QT -= gui

#input
HEADERS += DummyStorage.h

SOURCES += DummyStorage.cpp

#clean
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

target.path = /opt/tests/buteo-syncfw/
INSTALLS += target


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
