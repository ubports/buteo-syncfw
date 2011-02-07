#include "SyncOnChange.h"
#include "SyncOnChangeScheduler.h"
#include "SyncProfile.h"
#include "StorageChangeNotifier.h"
#include "LogMacros.h"

using namespace Buteo;

SyncOnChange::SyncOnChange() :
iStorageChangeNotifier(new StorageChangeNotifier()),
iSOCScheduler(0)
{
    FUNCTION_CALL_TRACE;
}

SyncOnChange::~SyncOnChange()
{
    FUNCTION_CALL_TRACE;
    QStringList storages;

    disable();

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
    FUNCTION_CALL_TRACE;
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

void SyncOnChange::enable()
{
    FUNCTION_CALL_TRACE;
    if(iStorageChangeNotifier)
    {
        QStringList aFailedStorages;
        bool enabled = iStorageChangeNotifier->startListen(aFailedStorages);
        Q_UNUSED(enabled);
        for(QStringList::const_iterator failedStorageItr = aFailedStorages.constBegin();
            failedStorageItr != aFailedStorages.constEnd(); ++failedStorageItr)
        {
            cleanup(*failedStorageItr);
        }
        iStorageChangeNotifier->checkForChanges();
    }
}

void SyncOnChange::disable()
{
    FUNCTION_CALL_TRACE;
    iStorageChangeNotifier->stopListen();
}

void SyncOnChange::disableNext()
{
    FUNCTION_CALL_TRACE;
    iStorageChangeNotifier->stopListen(true);
}

void SyncOnChange::cleanup(const QString& aStorageName)
{
    FUNCTION_CALL_TRACE;
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
    FUNCTION_CALL_TRACE;
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
    FUNCTION_CALL_TRACE;
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
