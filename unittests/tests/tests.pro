include(tests_common.pri)
TEMPLATE = subdirs
SUBDIRS = \
        msyncdtests \
        pluginmanagertests \
        syncfwclienttests \
        syncprofiletests \

# install
testwrapper.files = runstarget.sh
testwrapper.path = $${INSTALL_TESTDIR}
INSTALLS += testwrapper
testdefinition.files = tests.xml
testdefinition.path = $${INSTALL_TESTDIR}
INSTALLS += testdefinition
