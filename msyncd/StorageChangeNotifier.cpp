#include "StorageChangeNotifier.h"
#include "StorageChangeNotifierPlugin.h"
#include "PluginManager.h"

#include <QStringList>

using namespace Buteo;

StorageChangeNotifier::StorageChangeNotifier()
{
}

StorageChangeNotifier::~StorageChangeNotifier()
{
}

void StorageChangeNotifier::loadNotifiers(PluginManager* aPluginManager,
                                          const QStringList& aStorageNames)
{
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
    QObject::disconnect();
}

void StorageChangeNotifier::storageChanged()
{
    StorageChangeNotifierPlugin* plugin = qobject_cast<StorageChangeNotifierPlugin*>(sender());
    if(plugin)
    {
        emit storageChange(plugin->name());
    }
}
