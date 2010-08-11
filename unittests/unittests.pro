TEMPLATE = subdirs
SUBDIRS += dummyplugins \
           tests

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
