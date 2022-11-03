TEMPLATE = subdirs
SUBDIRS = \
        ClientPluginTest.pro \
        DeletedItemsIdStorageTest.pro \
        ServerPluginTest.pro \
        StoragePluginTest.pro \

coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
