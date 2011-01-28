#include "StorageChangeNotifier.h"
#include "StorageChangeNotifierPlugin.h"
#include "PluginManager.h"
#include "LogMacros.h"

#include <QStringList>

using namespace Buteo;

StorageChangeNotifier::StorageChangeNotifier()
{
    FUNCTION_CALL_TRACE;
}

StorageChangeNotifier::~StorageChangeNotifier()
{
    FUNCTION_CALL_TRACE;
}

void StorageChangeNotifier::loadNotifiers(PluginManager* aPluginManager,
                                          const QStringList& aStorageNames)
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = 0;

    for(QStringList::const_iterator storageNameItr = aStorageNames.constBegin();
        storageNameItr != aStorageNames.constEnd(); ++storageNameItr)
    {
        plugin = aPluginManager->createStorageChangeNotifier(*storageNameItr);
        iNotifierMap[*storageNameItr] = plugin;
    }
}

bool StorageChangeNotifier::startListen(QStringList& aFailedStorages)
{
    FUNCTION_CALL_TRACE;
    bool success = true;
    StorageChangeNotifierPlugin* plugin = 0;

    if(!iNotifierMap.count())
    {
        success = false;
    }
    for(QHash<QString,StorageChangeNotifierPlugin*>::iterator storageNameItr = iNotifierMap.begin();
        storageNameItr != iNotifierMap.end(); ++storageNameItr)
    {
        plugin = storageNameItr.value();
        if(plugin)
        {
            QObject::connect(plugin, SIGNAL(storageChange()),
                             this, SLOT(storageChanged()));
        }
        else
        {
            aFailedStorages << storageNameItr.key();
            success = false;
        }
    }
    return success;
}

void StorageChangeNotifier::stopListen()
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = 0;
    this->disconnect();
    for(QHash<QString,StorageChangeNotifierPlugin*>::iterator storageNameItr = iNotifierMap.begin();
        storageNameItr != iNotifierMap.end(); ++storageNameItr)
    {
        plugin = storageNameItr.value();
        if(plugin)
        {
            QObject::disconnect(plugin, 0, this, 0);
        }
    }
}

void StorageChangeNotifier::storageChanged()
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = qobject_cast<StorageChangeNotifierPlugin*>(sender());
    if(plugin)
    {
        LOG_DEBUG("Change in storage" << plugin->name());
        plugin->changesReceived();
        emit storageChange(plugin->name());
    }
}

void StorageChangeNotifier::checkForChanges()
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = 0;
    for(QHash<QString,StorageChangeNotifierPlugin*>::iterator storageNameItr = iNotifierMap.begin();
        storageNameItr != iNotifierMap.end(); ++storageNameItr)
    {
        plugin = storageNameItr.value();
        if(plugin && plugin->hasChanges())
        {
            plugin->changesReceived();
            emit storageChange(plugin->name());
        }
    }
}
