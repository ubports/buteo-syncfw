/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2014-2016 Jolla Ltd.
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#ifdef USE_KEEPALIVE
#include "BackgroundSync.h"
#else
#include "IPHeartBeat.h"
#endif
#include "SyncScheduler.h"
#include "SyncProfile.h"
#include "SyncCommonDefs.h"
#include "LogMacros.h"
#include <QtDBus/QtDBus>


using namespace Buteo;

SyncScheduler::SyncScheduler(QObject *aParent)
    :   QObject(aParent)
{
    FUNCTION_CALL_TRACE;

#ifdef USE_KEEPALIVE
    iBackgroundActivity = new BackgroundSync(this);

    connect(iBackgroundActivity, SIGNAL(onBackgroundSyncRunning(QString)), this, SLOT(doIPHeartbeatActions(QString)));
    connect(iBackgroundActivity, SIGNAL(onBackgroundSwitchRunning(QString)), this,
            SLOT(rescheduleBackgroundActivity(QString)));
#else
    iIPHeartBeatMan = new IPHeartBeat(this);

    connect(iIPHeartBeatMan, SIGNAL(onHeartBeat(QString)), this, SLOT(doIPHeartbeatActions(QString)));

    // Create the alarm inventory object
    iAlarmInventory = new SyncAlarmInventory();
    connect(iAlarmInventory, SIGNAL(triggerAlarm(int)),
            this, SLOT(doAlarmActions(int)));
    if (!iAlarmInventory->init()) {
        LOG_WARNING("AlarmInventory Init Failed");
    }

#endif
}

SyncScheduler::~SyncScheduler()
{
    FUNCTION_CALL_TRACE;

#ifdef USE_KEEPALIVE
    iBackgroundActivity->removeAll();
#else
    removeAllAlarms();
    if (iAlarmInventory) {
        delete iAlarmInventory;
        iAlarmInventory = 0;
    }
#endif
}

void SyncScheduler::addProfileForSyncRetry(const SyncProfile *aProfile, QDateTime aNextSyncTime)
{
    FUNCTION_CALL_TRACE;

    if (aProfile && aProfile->isEnabled()) {
#ifdef USE_KEEPALIVE
        setNextAlarm(aProfile, aNextSyncTime);
#else
        //remove alarm
        removeProfile(aProfile->name());

        int alarmId = setNextAlarm(aProfile, aNextSyncTime);
        if (alarmId > 0) {
            iSyncScheduleProfiles.insert(aProfile->name(), alarmId);
            LOG_DEBUG("syncretries : retry scheduled for profile" << aProfile->name());
        }
#endif
    }
}

bool SyncScheduler::addProfile(const SyncProfile *aProfile)
{
    FUNCTION_CALL_TRACE;

    if (!aProfile)
        return false;

// In case Keepalive is used no need to remove
// existent profile will be updated
#ifdef USE_KEEPALIVE
    if (aProfile->isEnabled() &&
            aProfile->syncType() == SyncProfile::SYNC_SCHEDULED) {
        setNextAlarm(aProfile);
        return true;
    } else {
        removeProfile(aProfile->name());
        return false;
    }

#else
    bool profileAdded = false;

    // Remove possible old alarm first.
    removeProfile(aProfile->name());

    if (aProfile->isEnabled() &&
            aProfile->syncType() == SyncProfile::SYNC_SCHEDULED) {
        int alarmId = setNextAlarm(aProfile);
        if (alarmId > 0) {
            iSyncScheduleProfiles.insert(aProfile->name(), alarmId);
            profileAdded = true;
            LOG_DEBUG("Sync scheduled: profile =" << aProfile->name() <<
                      "time =" << aProfile->nextSyncTime());
        } // no else
    } // no else

    return profileAdded;
#endif
}

void SyncScheduler::removeProfile(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;
#ifdef USE_KEEPALIVE
    if (iBackgroundActivity->remove(aProfileName)) {
        LOG_DEBUG("Scheduled sync removed: profile =" << aProfileName);
    }
#else
    if (iSyncScheduleProfiles.contains(aProfileName)) {
        int alarmEventID = iSyncScheduleProfiles.value(aProfileName);
        removeAlarmEvent(alarmEventID);
        iSyncScheduleProfiles.remove(aProfileName);
        LOG_DEBUG("Scheduled sync removed: profile =" << aProfileName);
    }
#endif
}

void SyncScheduler::doIPHeartbeatActions(QString aProfileName)
{
    FUNCTION_CALL_TRACE;
    iActiveBackgroundSyncProfiles.insert(aProfileName);
    emit syncNow(aProfileName);
}

void SyncScheduler::syncStatusChanged(const QString &aProfileName, int aStatus,
                                      const QString &aMessage, int aMoreDetails)
{
    if (iActiveBackgroundSyncProfiles.contains(aProfileName) && aStatus >= Sync::SYNC_ERROR) {
        // the background sync cycle is finished.
        // tell the scheduler that it can stop preventing device suspend.
        LOG_DEBUG("Background sync" << aProfileName << "finished with status:" << aStatus <<
                  "and extra:" << aMessage << "," << aMoreDetails);
        iActiveBackgroundSyncProfiles.remove(aProfileName);
#ifdef USE_KEEPALIVE
        iBackgroundActivity->onBackgroundSyncCompleted(aProfileName);
#endif

        // and schedule the next background sync if necessary.
        SyncProfile *profile = iProfileManager.syncProfile(aProfileName);
        if (profile) {
            setNextAlarm(profile);
            delete profile;
        }
    }
}

#ifdef USE_KEEPALIVE
void SyncScheduler::rescheduleBackgroundActivity(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;

    SyncProfile *profile = iProfileManager.syncProfile(aProfileName);
    if (profile) {
        if (profile->syncExternallyEnabled() || profile->syncExternallyDuringRush()) {
            emit externalSyncChanged(profile->name(), false);
        }
        setNextAlarm(profile);
        delete profile;
    } else {
        LOG_WARNING("Invalid profile, can't reschedule switch timer for " << aProfileName);
    }
}
#endif

int SyncScheduler::setNextAlarm(const SyncProfile *aProfile, QDateTime aNextSyncTime)
{
    FUNCTION_CALL_TRACE;

    int alarmEventID = -1;

    if (aProfile == 0) {
        return alarmEventID;
    }

    QDateTime nextSyncTime;
    if (!aNextSyncTime.isValid()) {
        nextSyncTime = aProfile->nextSyncTime(aProfile->lastSyncTime());
    } else {
        nextSyncTime = aNextSyncTime;
    }

    if (nextSyncTime.isValid()) {
        // The existing event object can be used by just updating the alarm time
        // and enqueuing it again.

#ifdef USE_KEEPALIVE
        alarmEventID = 1;
        iBackgroundActivity->set(aProfile->name(), QDateTime::currentDateTime().secsTo(nextSyncTime) + 1);

        if (aProfile->rushEnabled()) {
            QDateTime nextSyncSwitch = aProfile->nextRushSwitchTime(QDateTime::currentDateTime());
            if (nextSyncSwitch.isValid()) {
                iBackgroundActivity->setSwitch(aProfile->name(), nextSyncSwitch);
            } else {
                iBackgroundActivity->removeSwitch(aProfile->name());
                LOG_DEBUG("Removing switch timer for"
                          << aProfile->name() << " invalid switch timer");
            }
        } else {
            iBackgroundActivity->removeSwitch(aProfile->name());
        }
#else
        iAlarmInventory->addAlarm(nextSyncTime);
#endif
        if (alarmEventID == 0) {
            LOG_WARNING("Failed to add alarm for scheduled sync of profile"
                        << aProfile->name());
        }
    } else {
#ifdef USE_KEEPALIVE
        // no valid next scheduled sync time for background sync.
        // stop the background activity to allow device suspend.
        iBackgroundActivity->remove(aProfile->name());
        if (aProfile->rushEnabled()) {
            QDateTime nextSyncSwitch = aProfile->nextRushSwitchTime(QDateTime::currentDateTime());
            if (nextSyncSwitch.isValid()) {
                iBackgroundActivity->setSwitch(aProfile->name(), nextSyncSwitch);
            } else {
                iBackgroundActivity->removeSwitch(aProfile->name());
            }
        } else {
            iBackgroundActivity->removeSwitch(aProfile->name());
        }
#endif
        LOG_WARNING("Next sync time is not valid, sync not scheduled for profile"
                    << aProfile->name());
    }

    return alarmEventID;
}

#ifndef USE_KEEPALIVE
void SyncScheduler::doAlarmActions(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE;

    const QString syncProfileName
        = iSyncScheduleProfiles.key(aAlarmEventID);

    if (!syncProfileName.isEmpty()) {
        iSyncScheduleProfiles.remove(syncProfileName);
        // Use global slots (min time == max time) for scheduling heart beats.
        if (iIPHeartBeatMan->setHeartBeat(syncProfileName, IPHB_GS_WAIT_2_5_MINS, IPHB_GS_WAIT_2_5_MINS)) {
            //Do nothing, sync will be triggered on getting heart beat
        } else {
            emit syncNow(syncProfileName);
        }
    } // no else, in error cases simply ignore

}

void SyncScheduler::removeAlarmEvent(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE;

    bool err = iAlarmInventory->removeAlarm(aAlarmEventID);

    if (err < false) {
        LOG_WARNING("No alarm found for ID " << aAlarmEventID);
    } else {
        LOG_DEBUG("Removed alarm, ID =" << aAlarmEventID);
    }
}

void SyncScheduler::removeAllAlarms()
{
    FUNCTION_CALL_TRACE;

    iAlarmInventory->removeAllAlarms();
}
#endif
