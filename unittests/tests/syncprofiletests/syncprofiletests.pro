include(../tests_common.pri)
TEMPLATE = subdirs
SUBDIRS = \
        ProfileFactoryTest \
        ProfileFieldTest \
        ProfileManagerTest \
        ProfileTest \
        StorageProfileTest \
        SyncLogTest \
        SyncProfileTest \
        SyncScheduleTest \

testprofiles_client.files = testprofiles/user/client/*
testprofiles_client.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/client/

testprofiles_storage.files = testprofiles/user/storage/*
testprofiles_storage.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/storage/

testprofiles_sync.files = testprofiles/user/sync/*
testprofiles_sync.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/user/sync/

testprofiles_syssync.files = testprofiles/system/sync/*
testprofiles_syssync.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/system/sync/

testprofiles_expstorage.files = testprofiles/expected/storage/*
testprofiles_expstorage.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/expected/storage/

testprofiles_expsync.files = testprofiles/expected/sync/*
testprofiles_expsync.path = $${INSTALL_TESTDIR}/syncprofiletests/testprofiles/expected/sync/

testaccount_service.files = testprofiles/testsync-ovi.service
testaccount_service.path = /usr/share/accounts/services/

INSTALLS += \
    testaccount_service \
    testprofiles_client \
    testprofiles_storage \
    testprofiles_sync \
    testprofiles_syssync \
    testprofiles_expstorage \
    testprofiles_expsync \
