/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "SyncScheduler.h"
#include "IPHeartBeat.h"
#include "SyncProfile.h"
#include "LogMacros.h"
#include <QtDBus/QtDBus>
#include "IPHeartBeat.h"

using namespace Buteo;


SyncScheduler::SyncScheduler(QObject *aParent)
:   QObject(aParent)
{
    FUNCTION_CALL_TRACE;
    
    iIPHeartBeatMan = new IPHeartBeat(this);
    
    connect(iIPHeartBeatMan,SIGNAL(onHeartBeat(QString)),this,SLOT(doIPHeartbeatActions(QString)));

    // Create the alarm inventory object
    iAlarmInventory = new SyncAlarmInventory();
    if(iAlarmInventory) {
    	connect ( iAlarmInventory, SIGNAL(triggerAlarm(int)),
              this, SLOT(doAlarmActions(int)) );
    	if(!iAlarmInventory->init()) {
    		LOG_WARNING("AlarmInventory Init Failed");
    	}
    }

}

SyncScheduler::~SyncScheduler() 
{
    FUNCTION_CALL_TRACE;
    
    removeAllAlarms();
    if (iAlarmInventory) {
        delete iAlarmInventory;
        iAlarmInventory = 0;
    }
}
    
void SyncScheduler::addProfileForSyncRetry(const SyncProfile* aProfile, QDateTime aNextSyncTime)
{
    FUNCTION_CALL_TRACE;
    if(aProfile && aProfile->isEnabled())
    {
        removeProfile(aProfile->name());
        int alarmId = setNextAlarm(aProfile, aNextSyncTime);
        if (alarmId > 0)
        {
            iSyncScheduleProfiles.insert(aProfile->name(), alarmId);
            LOG_DEBUG("syncretries : retry scheduled for profile" << aProfile->name());
        }
    }
}

bool SyncScheduler::addProfile(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;

    if (!aProfile)
        return false;
    
    bool profileAdded = false;

    // Remove possible old alarm first.
    removeProfile(aProfile->name());
    
    if (aProfile->isEnabled() &&
        aProfile->syncType() == SyncProfile::SYNC_SCHEDULED)
    {
        int alarmId = setNextAlarm(aProfile);
        if (alarmId > 0) {
            iSyncScheduleProfiles.insert(aProfile->name(), alarmId);
            profileAdded = true;
            //LOG_DEBUG("Sync scheduled: profile =" << aProfile->name() <<
            //    "time =" << aProfile->nextSyncTime());
        } // no else
    } // no else
    
    return profileAdded;
}


void SyncScheduler::removeProfile(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;
    
    if (iSyncScheduleProfiles.contains(aProfileName)) {
        int alarmEventID = iSyncScheduleProfiles.value(aProfileName);
        removeAlarmEvent(alarmEventID);
        iSyncScheduleProfiles.remove(aProfileName);
        LOG_DEBUG("Scheduled sync removed: profile =" << aProfileName);
    }
}

void SyncScheduler::doIPHeartbeatActions(QString aProfileName)
{
    FUNCTION_CALL_TRACE;
    
    emit syncNow(aProfileName);
}


int SyncScheduler::setNextAlarm(const SyncProfile* aProfile, QDateTime aNextSyncTime)
{
    FUNCTION_CALL_TRACE;
    
    int alarmEventID = -1;

    if(aProfile == 0)
    {
        return alarmEventID;
    }

    QDateTime nextSyncTime;
    if(!aNextSyncTime.isValid())
    {
        nextSyncTime = aProfile->nextSyncTime(aProfile->lastSyncTime());
    }
    else
    {
        nextSyncTime = aNextSyncTime;
    }
    
    if (nextSyncTime.isValid()) {
        // The existing event object can be used by just updating the alarm time
        // and enqueuing it again.
        
        alarmEventID = iAlarmInventory->addAlarm(nextSyncTime);
        if (alarmEventID == 0)
        {
            LOG_WARNING("Failed to add alarm for scheduled sync of profile"
                << aProfile->name());
        }
    }
    else {
        LOG_WARNING("Next sync time is not valid, sync not scheduled for profile"
            << aProfile->name());
    }
    
    return alarmEventID;
}


void SyncScheduler::doAlarmActions(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE;
    
    const QString syncProfileName
        = iSyncScheduleProfiles.key(aAlarmEventID);
    
    if (!syncProfileName.isEmpty()) {
        iSyncScheduleProfiles.remove(syncProfileName);
        // Use global slots (min time == max time) for scheduling heart beats.
        if(iIPHeartBeatMan->setHeartBeat(syncProfileName, IPHB_GS_WAIT_2_5_MINS, IPHB_GS_WAIT_2_5_MINS)) {
            //Do nothing, sync will be triggered on getting heart beat
        } else {
            emit syncNow(syncProfileName);
        }
    } // no else, in error cases simply ignore
    
}


void SyncScheduler::removeAlarmEvent(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE;
    
    bool err = iAlarmInventory->removeAlarm( aAlarmEventID );
    
    if (err < false) {
        LOG_WARNING("No alarm found for ID " << aAlarmEventID);
    }
    else
    {
        LOG_DEBUG("Removed alarm, ID =" << aAlarmEventID);
    }
}


void SyncScheduler::removeAllAlarms()
{
    FUNCTION_CALL_TRACE;

    iAlarmInventory->removeAllAlarms();    
}

