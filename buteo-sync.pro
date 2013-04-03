include(doc/doc.pro)

TEMPLATE = subdirs

SUBDIRS += libbuteosyncfw \
           msyncd \
           unittests \
	   doc

QT += testlib

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
