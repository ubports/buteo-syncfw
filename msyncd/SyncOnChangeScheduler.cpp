#include <QTimer>

#include "SyncOnChangeScheduler.h"
#include "SyncProfile.h"

using namespace Buteo;

SyncOnChangeScheduler::SyncOnChangeScheduler() :
iTimeout(SOC_SCHEDULE_TIMEOUT),
iSOCScheduleCriterion(USE_TIMEOUT)
{
}

SyncOnChangeScheduler::~SyncOnChangeScheduler()
{
}

bool SyncOnChangeScheduler::addProfile(const SyncProfile* aProfile)
{
    bool scheduled = false;
    // Only timeout and sync now criterion supported as of now
    if(USE_TIMEOUT == iSOCScheduleCriterion &&
       aProfile && !iSOCProfileNames.contains(aProfile->name()))
    {
        iSOCProfileNames << aProfile->name();
        SyncOnChangeTimer *SOCtimer = new SyncOnChangeTimer(aProfile, iTimeout);
        QObject::connect(SOCtimer, SIGNAL(timeout(SyncProfile*)),
                         this, SLOT(scheduleSync(SyncProfile*)));
        SOCtimer->fire();
        scheduled = true;
    }
    else if(SYNC_NOW == iSOCScheduleCriterion &&
             aProfile)
    {
        emit syncNow(aProfile->name());
        scheduled = true;
    }
    return scheduled;
}

void SyncOnChangeScheduler::removeProfile(const QString &aProfileName)
{
    iSOCProfileNames.removeAll(aProfileName);
}

void SyncOnChangeScheduler::setTimeout(const quint32& aTimeout)
{
    iTimeout = aTimeout;
}

void SyncOnChangeScheduler::setSOCScheduleCriterion(SOCScheduleCriteria aSOCScheduleCriterion)
{
    if(aSOCScheduleCriterion > NO_SCHEDULE &&
       aSOCScheduleCriterion < NUMBER_OF_CRITERIA)
    {
        iSOCScheduleCriterion = aSOCScheduleCriterion;
    }
    // else silently ignore
}

void SyncOnChangeScheduler::scheduleSync(const SyncProfile* aProfile)
{
    iSOCProfileNames.removeAll(aProfile->name());
    SyncOnChangeTimer *SOCtimer = qobject_cast<SyncOnChangeTimer*>(sender());
    if(SOCtimer)
    {
        delete SOCtimer;
        emit syncNow(aProfile->name());
    }
}

SyncOnChangeTimer::SyncOnChangeTimer(const SyncProfile* profile, const quint32& aTimeout) :
iSyncProfile(profile), iTimeout(aTimeout)
{
}

SyncOnChangeTimer::~SyncOnChangeTimer()
{
}

void SyncOnChangeTimer::fire()
{
    QTimer::singleShot(iTimeout*1000, this, SLOT(onTimeout()));
}

void SyncOnChangeTimer::onTimeout()
{
    emit timeout(iSyncProfile);
}
