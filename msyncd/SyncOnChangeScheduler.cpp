#include <QTimer>

#include "SyncOnChangeScheduler.h"
#include "SyncProfile.h"
#include "LogMacros.h"

using namespace Buteo;

SyncOnChangeScheduler::SyncOnChangeScheduler() :
iDefaultSOCAfterTime(DEFAULT_SOC_AFTER_TIME)
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
    if(aProfile && !iSOCProfileNames.contains(aProfile->name()))
    {
        qint32 time = aProfile->syncOnChangeAfter();
        time = time != -1 ? time : iDefaultSOCAfterTime;
        iSOCProfileNames << aProfile->name();
        SyncOnChangeTimer *SOCtimer = new SyncOnChangeTimer(aProfile, time);
        QObject::connect(SOCtimer, SIGNAL(timeout(const SyncProfile*)),
                         this, SLOT(sync(const SyncProfile*)),
                         Qt::QueuedConnection);
        SOCtimer->fire();
        scheduled = true;
        LOG_DEBUG("Sync on change scheduled for profile"<< aProfile->name());
    }
    else if(aProfile)
    {
        LOG_DEBUG("Sync on change couldn't be scheduled for profile" << aProfile->name());
    }
    return scheduled;
}

void SyncOnChangeScheduler::removeProfile(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;
    iSOCProfileNames.removeAll(aProfileName);
}

void SyncOnChangeScheduler::setDefaultSOCAfterTime(const quint32& aTime)
{
    FUNCTION_CALL_TRACE;
    iDefaultSOCAfterTime = aTime;
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
