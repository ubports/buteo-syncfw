TEMPLATE = subdirs

SUBDIRS += \
    libbuteosyncfw \
    msyncd \
    oopp-runner \
    unittests \
    doc

msyncd.depends = libbuteosyncfw
oopp-runner.depends = libbuteosyncfw
unittests.depends = libbuteosyncfw msyncd

coverage.CONFIG += recursive

QMAKE_EXTRA_TARGETS += coverage
