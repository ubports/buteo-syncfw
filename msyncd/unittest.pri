CONFIG += link_prl
msyncd_out_pwd = $${OUT_PWD}/$$relative_path($${PWD}, $$dirname(_PRO_FILE_))
LIBS += -L$${msyncd_out_pwd} -lmsyncd
SOURCES += $$PWD/UnitTest.cpp
# Already present in ../unittests/tests/tests.pro
#DEFINES += SYNCFW_UNIT_TESTS
