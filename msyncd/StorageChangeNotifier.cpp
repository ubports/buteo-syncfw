#include "StorageChangeNotifier.h"
#include "StorageChangeNotifierPlugin.h"
#include "PluginManager.h"
#include "LogMacros.h"

#include <QStringList>

using namespace Buteo;

StorageChangeNotifier::StorageChangeNotifier() :
iPluginManager(0)
{
    FUNCTION_CALL_TRACE;
}

StorageChangeNotifier::~StorageChangeNotifier()
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = 0;
    for(QHash<QString,StorageChangeNotifierPlugin*>::iterator storageNameItr = iNotifierMap.begin();
        storageNameItr != iNotifierMap.end(); ++storageNameItr)
    {
        plugin = storageNameItr.value();
        if(iPluginManager && plugin)
        {
            iPluginManager->destroyStorageChangeNotifier(plugin);
        }
    }
}

void StorageChangeNotifier::loadNotifiers(PluginManager* aPluginManager,
                                          const QStringList& aStorageNames)
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = 0;
    iPluginManager = aPluginManager;
    for(QStringList::const_iterator storageNameItr = aStorageNames.constBegin();
        storageNameItr != aStorageNames.constEnd(); ++storageNameItr)
    {
        if(iPluginManager)
        {
            plugin = iPluginManager->createStorageChangeNotifier(*storageNameItr);
            if(plugin)
            {
                iNotifierMap[*storageNameItr] = plugin;
            }
        }
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
            plugin->enable();
        }
        else
        {
            aFailedStorages << storageNameItr.key();
            success = false;
        }
    }
    return success;
}

void StorageChangeNotifier::stopListen(bool disableAfterNextChange)
{
    FUNCTION_CALL_TRACE;
    StorageChangeNotifierPlugin* plugin = 0;
    for(QHash<QString,StorageChangeNotifierPlugin*>::iterator storageNameItr = iNotifierMap.begin();
        storageNameItr != iNotifierMap.end(); ++storageNameItr)
    {
        plugin = storageNameItr.value();
        if(plugin)
        {
            QObject::disconnect(plugin, SIGNAL(storageChange()),
                                this, SLOT(storageChanged()));
            plugin->disable(disableAfterNextChange);
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
