TEMPLATE = subdirs

SUBDIRS += \
    libbuteosyncfw \
    msyncd \
    unittests \
    tools \
    doc

msyncd.depends = libbuteosyncfw
unittests.depends = libbuteosyncfw msyncd

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
