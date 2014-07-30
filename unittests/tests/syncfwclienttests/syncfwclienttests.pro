TEMPLATE = subdirs
SUBDIRS = \
        SyncClientInterfaceTest.pro \

coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
