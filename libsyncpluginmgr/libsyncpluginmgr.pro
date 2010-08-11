TEMPLATE = lib

VER_MAJ = 0
VER_MIN = 4
VER_PAT = 4


TARGET = syncpluginmgr

DEPENDPATH += .

INCLUDEPATH += . \
    .. \
    ../libsynccommon \
    ../libsyncprofile

QMAKE_LFLAGS += -ldl

CONFIG += dll debug silent

QT+=sql
QT -= gui


LIBS += -L../libsyncprofile/ \
        -lsyncprofile 

# Input
HEADERS += ClientPlugin.h \
	PluginCbInterface.h \
	PluginManager.h \
	ServerPlugin.h \
	StorageItem.h \
	StoragePlugin.h \
	SyncPluginBase.h \
    DeletedItemsIdStorage.h   

SOURCES += ClientPlugin.cpp \
	PluginManager.cpp \
	ServerPlugin.cpp \
	StorageItem.cpp \
	StoragePlugin.cpp \
	SyncPluginBase.cpp \
    DeletedItemsIdStorage.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions



# clean
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)

QMAKE_CLEAN += $(OBJECTS_DIR)/moc_*

# install
target.path = /usr/lib/
headers.path = /usr/include/libsyncpluginmgr
headers.files = ClientPlugin.h \
    PluginCbInterface.h \
    PluginManager.h \
    ServerPlugin.h \
    StorageItem.h \
    StoragePlugin.h \
    SyncPluginBase.h \
    DeletedItemsIdStorage.h
INSTALLS += target headers

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
