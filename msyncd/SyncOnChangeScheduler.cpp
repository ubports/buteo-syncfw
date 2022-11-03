#include <QTimer>

#include "SyncOnChangeScheduler.h"
#include "SyncProfile.h"
#include "LogMacros.h"

using namespace Buteo;

SyncOnChangeScheduler::SyncOnChangeScheduler()
{
    FUNCTION_CALL_TRACE;
}

SyncOnChangeScheduler::~SyncOnChangeScheduler()
{
    FUNCTION_CALL_TRACE;
    foreach(QObject *o, iSOCTimers.values()) {
        delete o;
    }
    iSOCTimers.clear();
    iSOCProfileNames.clear();
}

bool SyncOnChangeScheduler::addProfile(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    bool scheduled = false;
    if(aProfile && !iSOCProfileNames.contains(aProfile->name()))
    {
        qint32 time = aProfile->syncOnChangeAfter();
        iSOCProfileNames << aProfile->name();
        SyncOnChangeTimer *SOCtimer = new SyncOnChangeTimer(aProfile, time);
        QObject::connect(SOCtimer, SIGNAL(timeout(const SyncProfile*)),
                         this, SLOT(sync(const SyncProfile*)),
                         Qt::QueuedConnection);
        SOCtimer->fire();
        scheduled = true;
        iSOCTimers.insert(aProfile->name(), SOCtimer);
        LOG_DEBUG("Sync on change scheduled for profile"<< aProfile->name());
    }
    else if(aProfile)
    {
        LOG_DEBUG("Sync on change already scheduled for profile" << aProfile->name());
    }
    return scheduled;
}

void SyncOnChangeScheduler::removeProfile(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;
    iSOCProfileNames.removeAll(aProfileName);
    // cancel timer
    delete iSOCTimers.take(aProfileName);
}

void SyncOnChangeScheduler::sync(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    iSOCProfileNames.removeAll(aProfile->name());
    iSOCTimers.remove(aProfile->name());
    SyncOnChangeTimer *SOCtimer = qobject_cast<SyncOnChangeTimer*>(sender());
    if(SOCtimer)
    {
        LOG_DEBUG("Sync on change for profile" << aProfile->name());
        delete SOCtimer;
        emit syncNow(aProfile->name());
    }
}

SyncOnChangeTimer::SyncOnChangeTimer(const SyncProfile* profile, const quint32& aTimeout) :
iSyncProfile(profile), iTimeout(aTimeout)
{
    FUNCTION_CALL_TRACE;
}

SyncOnChangeTimer::~SyncOnChangeTimer()
{
    FUNCTION_CALL_TRACE;
}

void SyncOnChangeTimer::fire()
{
    FUNCTION_CALL_TRACE;
    QTimer::singleShot(iTimeout*1000, this, SLOT(onTimeout()));
}

void SyncOnChangeTimer::onTimeout()
{
    FUNCTION_CALL_TRACE;
    emit timeout(iSyncProfile);
}
