TEMPLATE = subdirs
SUBDIRS = \
        AccountsHelperTest \
        ClientPluginRunnerTest \
        ClientThreadTest \
        PluginRunnerTest \
        ServerActivatorTest \
        ServerPluginRunnerTest \
        ServerThreadTest \
        StorageBookerTest \
        SyncBackupTest \
        SyncQueueTest \
        SyncSessionTest \
        SyncSigHandlerTest \
        SynchronizerTest \
        TransportTrackerTest \

!contains(DEFINES, USE_KEEPALIVE) {
SUBDIRS += \
        IPHeartBeatTest \
        SyncSchedulerTest \
}

coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
