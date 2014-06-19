include(../tests_common.pri)
TEMPLATE = subdirs
SUBDIRS = \
        ProfileFactoryTest.pro \
        ProfileFieldTest.pro \
        ProfileManagerTest.pro \
        ProfileTest.pro \
        StorageProfileTest.pro \
        SyncLogTest.pro \
        SyncProfileTest.pro \
        SyncScheduleTest.pro \

testprofiles_client.files = testprofiles/user/client/*
testprofiles_client.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/client/

testprofiles_service.files = testprofiles/user/service/*
testprofiles_service.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/service/

testprofiles_storage.files = testprofiles/user/storage/*
testprofiles_storage.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/storage/

testprofiles_sync.files = testprofiles/user/sync/*
testprofiles_sync.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/sync/

testprofiles_syssync.files = testprofiles/system/sync/*
testprofiles_syssync.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/system/sync/

testaccount_service.files = testprofiles/testsync-ovi.service
testaccount_service.path = /usr/share/accounts/services/

INSTALLS += \
    testaccount_service \
    testprofiles_client \
    testprofiles_service \
    testprofiles_storage \
    testprofiles_sync \
    testprofiles_syssync \
