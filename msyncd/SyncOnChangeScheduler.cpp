#include <QTimer>

#include "SyncOnChangeScheduler.h"
#include "SyncProfile.h"
#include "LogMacros.h"

using namespace Buteo;

SyncOnChangeScheduler::SyncOnChangeScheduler() :
iTimeout(SOC_SCHEDULE_TIMEOUT),
iSOCScheduleCriterion(USE_TIMEOUT)
{
    FUNCTION_CALL_TRACE;
}

SyncOnChangeScheduler::~SyncOnChangeScheduler()
{
    FUNCTION_CALL_TRACE;
}

bool SyncOnChangeScheduler::addProfile(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    bool scheduled = false;
    // Only timeout and sync now criterion supported as of now
    if(USE_TIMEOUT == iSOCScheduleCriterion &&
       aProfile && !iSOCProfileNames.contains(aProfile->name()))
    {
        iSOCProfileNames << aProfile->name();
        SyncOnChangeTimer *SOCtimer = new SyncOnChangeTimer(aProfile, iTimeout);
        QObject::connect(SOCtimer, SIGNAL(timeout(SyncProfile*)),
                         this, SLOT(sync(SyncProfile*)));
        SOCtimer->fire();
        scheduled = true;
        LOG_DEBUG("Sync on change scheduled for profile"<< aProfile->name());
    }
    else if(SYNC_NOW == iSOCScheduleCriterion &&
             aProfile)
    {
        emit syncNow(aProfile->name());
        scheduled = true;
    }
    else
    {
        LOG_DEBUG("Sync on change couldn't be scheduled for profile"<< aProfile->name());
    }
    return scheduled;
}

void SyncOnChangeScheduler::removeProfile(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;
    iSOCProfileNames.removeAll(aProfileName);
}

void SyncOnChangeScheduler::setTimeout(const quint32& aTimeout)
{
    FUNCTION_CALL_TRACE;
    iTimeout = aTimeout;
}

void SyncOnChangeScheduler::setSOCScheduleCriterion(SOCScheduleCriteria aSOCScheduleCriterion)
{
    FUNCTION_CALL_TRACE;
    if(aSOCScheduleCriterion > NO_SCHEDULE &&
       aSOCScheduleCriterion < NUMBER_OF_CRITERIA)
    {
        iSOCScheduleCriterion = aSOCScheduleCriterion;
    }
    // else silently ignore
}

void SyncOnChangeScheduler::sync(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    iSOCProfileNames.removeAll(aProfile->name());
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
