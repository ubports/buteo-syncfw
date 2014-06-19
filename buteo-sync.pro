include(doc/doc.pro)

TEMPLATE = subdirs

SUBDIRS += \
    libbuteosyncfw \
    msyncd \
    unittests \
    doc \
    tools

msyncd.depends = libbuteosyncfw
unittests.depends = libbuteosyncfw msyncd

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
