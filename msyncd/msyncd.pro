TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = msyncd-lib.pro msyncd-app.pro

# #####################################################################
# make coverage (debug)
# #####################################################################
coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
