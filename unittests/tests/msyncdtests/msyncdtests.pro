TEMPLATE = subdirs
SUBDIRS = \
        AccountsHelperTest.pro \
        ClientPluginRunnerTest.pro \
        ClientThreadTest.pro \
        PluginRunnerTest.pro \
        ServerActivatorTest.pro \
        ServerPluginRunnerTest.pro \
        ServerThreadTest.pro \
        StorageBookerTest.pro \
        SyncBackupTest.pro \
        SyncQueueTest.pro \
        SyncSessionTest.pro \
        SyncSigHandlerTest.pro \
        SynchronizerTest.pro \
        TransportTrackerTest.pro \

!contains(DEFINES, USE_KEEPALIVE) {
SUBDIRS += \
        IPHeartBeatTest.pro \
        SyncSchedulerTest.pro \
}

coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
