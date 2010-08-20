include(doc/doc.pro)

TEMPLATE = subdirs

SUBDIRS += libsynccommon \
           libsyncprofile \
           libsyncpluginmgr \
           msyncd \
           libsyncfwclient \
           unittests \
	   doc

QT += testlib

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
