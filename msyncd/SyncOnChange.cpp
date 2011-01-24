#include "SyncOnChange.h"
#include "SyncOnChangeScheduler.h"
#include "SyncProfile.h"
#include "StorageChangeNotifier.h"

#include <QStringList>

using namespace Buteo;

SyncOnChange::SyncOnChange() :
iStorageChangeNotifier(new StorageChangeNotifier()),
iSOCScheduler(0)
{
}

SyncOnChange::~SyncOnChange()
{
    QStringList storages;

    storages = getSOCStorageNames();
    for(QStringList::const_iterator storageItr = storages.constBegin();
        storageItr != storages.constEnd(); ++storageItr)
    {
        cleanup(*storageItr);
    }
    delete iStorageChangeNotifier;
}

bool SyncOnChange::enable(const QHash<QString,QList<SyncProfile*> >& aSOCStorageMap,
                          SyncOnChangeScheduler* aSOCScheduler,
                          PluginManager* aPluginManager,
                          QStringList& aFailedStorages)
{
    bool enabled = false;
    QStringList storages;

    iSOCStorageMap = aSOCStorageMap;
    iSOCScheduler = aSOCScheduler;
    storages = getSOCStorageNames();
    iStorageChangeNotifier->loadNotifiers(aPluginManager, storages);
    enabled = iStorageChangeNotifier->startListen(aFailedStorages);
    for(QStringList::const_iterator failedStorageItr = aFailedStorages.constBegin();
        failedStorageItr != aFailedStorages.constEnd(); ++failedStorageItr)
    {
        cleanup(*failedStorageItr);
    }
    if(storages.count() > aFailedStorages.count())
    {
        QObject::connect(iStorageChangeNotifier, SIGNAL(storageChange(QString)),
                         this, SLOT(sync(QString)));
    }
    return enabled;
}

void SyncOnChange::disable()
{
    iStorageChangeNotifier->stopListen();
}

void SyncOnChange::cleanup(const QString& aStorageName)
{
    QList<SyncProfile*> profilesList;

    if(iSOCStorageMap.contains(aStorageName))
    {
        profilesList = iSOCStorageMap.value(aStorageName);
    }
    for(QList<SyncProfile*>::iterator profileItr = profilesList.begin();
        profileItr != profilesList.end(); ++profileItr)
    {
        delete (*profileItr);
    }
    iSOCStorageMap.remove(aStorageName);
}

QStringList SyncOnChange::getSOCStorageNames()
{
    QStringList storages;

    for(QHash<QString,QList<SyncProfile*> >::const_iterator storageNameItr = iSOCStorageMap.constBegin();
        storageNameItr != iSOCStorageMap.constEnd(); ++storageNameItr)
    {
        storages << storageNameItr.key();
    }
    return storages;
}

void SyncOnChange::sync(QString aStorageName)
{
    QList<SyncProfile*> profilesList;

    if(iSOCStorageMap.contains(aStorageName))
    {
        profilesList = iSOCStorageMap.value(aStorageName);
    }
    for(QList<SyncProfile*>::iterator profileItr = profilesList.begin();
        profileItr != profilesList.end(); ++profileItr)
    {
        iSOCScheduler->addProfile(*profileItr);
    }
}
