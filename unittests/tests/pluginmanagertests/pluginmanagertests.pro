TEMPLATE = subdirs
SUBDIRS = \
        ClientPluginTest \
        DeletedItemsIdStorageTest \
        ServerPluginTest \
        StoragePluginTest \

coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
