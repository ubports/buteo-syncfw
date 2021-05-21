TEMPLATE = subdirs
SUBDIRS = \
        SyncClientInterfaceTest \

coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
