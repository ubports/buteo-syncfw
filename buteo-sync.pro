#include(doc/doc.pro)

TEMPLATE = subdirs

SUBDIRS += libsynccommon \
           libsyncprofile \
           libsyncpluginmgr \
           msyncd
#           unittests

QT += testlib

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
