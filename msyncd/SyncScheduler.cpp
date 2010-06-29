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
#include "SyncSchedulerDBusAdaptor.h"
//#include "IPHeartBeat.h"
#include "SyncProfile.h"
//#include <alarmd/libalarm.h>
#include "LogMacros.h"
#include <QtDBus/QtDBus>

using namespace Buteo;

const char* SyncScheduler::DBUS_INTERFACE = "com.nokia.msyncd.scheduler";
const char* SyncScheduler::DBUS_SERVICE = "com.nokia.msyncd";
const char* SyncScheduler::DBUS_OBJECT = "/scheduler";
//const char* SyncScheduler::DBUS_NAME = "triggerAlarm";

const int MAX_IP_HEARTBEAT_WAIT_TIME = 120;

SyncScheduler::SyncScheduler(QObject *aParent)
:   QObject(aParent)
{
    FUNCTION_CALL_TRACE;
    
    //iIPHeartBeatMan = new IPHeartBeat(this);
    
    //connect(iIPHeartBeatMan,SIGNAL(onHeartBeat(QString)),this,SLOT(doIPHeartbeatActions(QString)));
    
    // Set up an alarm event for scheduled syncs. The existing event will be
    // updated and re-used when programming new alarms.
    //setupAlarmEvent( createAlarmEvent() );
    
    // Create a DBUS-adaptor for receiving trigger signals from alarmd
    //setupDBusAdaptor();

    // Create the alarm inventory object
    iAlarmInventory = new SyncAlarmInventory();
    connect ( iAlarmInventory, SIGNAL(triggerAlarm(int)), 
              this, SLOT(doAlarmActions(int)) );

}

SyncScheduler::~SyncScheduler() 
{
    FUNCTION_CALL_TRACE;
    
    removeAllAlarms();
    if (iAlarmInventory) {
        delete iAlarmInventory;
        iAlarmInventory = 0;
    }

    // Unregister from D-Bus.
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject(DBUS_OBJECT);
    LOG_DEBUG("Unregistered from D-Bus");
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


/*!
    \fn SyncScheduler::setNextAlarm()
 */
int SyncScheduler::setNextAlarm(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    
    int alarmEventID = -1;
    QDateTime nextSyncTime = aProfile->nextSyncTime();
    
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


/*!
    \fn SyncScheduler::setupAlarmActions()
 */
/*
void SyncScheduler::setupAlarmActions()
{
    FUNCTION_CALL_TRACE;
    
    // This function sets desired options for the alarm. 
    //      * Use DBUS as the notification method
    //      * Use DBUS signals
    //      * Notify when the alarm is triggered
    //      * Send a DBUS signal to slot triggerAlarm in SyncSchedulerDBusAdaptor
    
    const int NUMBEROFACTIONS = 1;
    iAlarmActionParameters = alarm_event_add_actions(iAlarmEventParameters, NUMBEROFACTIONS);
    
    if (iAlarmActionParameters != NULL) {
        unsigned actionMask = 0;
        actionMask |= ALARM_ACTION_TYPE_DBUS;
        actionMask |= ALARM_ACTION_WHEN_TRIGGERED;
        actionMask |= ALARM_ACTION_DBUS_ADD_COOKIE;
        iAlarmActionParameters->flags = actionMask;
        
        alarm_action_set_dbus_interface(iAlarmActionParameters, DBUS_INTERFACE);
        alarm_action_set_dbus_service(iAlarmActionParameters, DBUS_SERVICE);
        alarm_action_set_dbus_path(iAlarmActionParameters, DBUS_OBJECT);
        alarm_action_set_dbus_name(iAlarmActionParameters, DBUS_NAME);
    }
    else {
        LOG_WARNING("Failed to create alarm event actions");
    }
    
}
*/


/*!
    \fn SyncScheduler::setupAlarmEvent()
 */
/*
void SyncScheduler::setupAlarmEvent(alarm_event_t* aEvent)
{
    FUNCTION_CALL_TRACE;

    iAlarmEventParameters = aEvent;
    
    if (iAlarmEventParameters != NULL) {
    
        static const char* title = "Scheduled sync";
        alarm_event_set_title(iAlarmEventParameters, title);
        static const char* appID = "msyncd";
        alarm_event_set_alarm_appid(iAlarmEventParameters, appID);
        
        setupAlarmActions();
    }
    else {
        LOG_WARNING("Failed to create alarm event parameters");
    }

}
*/


/*!
    \fn SyncScheduler::doAlarmActions()
 */
void SyncScheduler::doAlarmActions(long aAlarmEventID)
{
    FUNCTION_CALL_TRACE;
    
    const QString syncProfileName
        = iSyncScheduleProfiles.key(aAlarmEventID);
    
    if (!syncProfileName.isEmpty()) {
        iSyncScheduleProfiles.remove(syncProfileName);
        //if(iIPHeartBeatMan->setHeartBeat(syncProfileName,0,MAX_IP_HEARTBEAT_WAIT_TIME) == true)
        if(0) {
            //Do nothing, sync will be triggered on getting heart beat
        } else {
            emit syncNow(syncProfileName);
        }
    } // no else, in error cases simply ignore
    
}


/*!
    \fn SyncScheduler::setupDBusAdaptor()
 */
void SyncScheduler::setupDBusAdaptor()
{
    FUNCTION_CALL_TRACE;
    
    new SyncSchedulerDBusAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    
    if (dbus.registerObject(DBUS_OBJECT, this)) {
        
        LOG_DEBUG("Registered sync scheduler to D-Bus");
    }
    else {
        LOG_CRITICAL("Failed to register sync scheduler to D-Bus");
    }
}


/*!
    \fn SyncScheduler::removeAlarmEvent(int aAlarmEventID)
 */
void SyncScheduler::removeAlarmEvent(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE;
    
    bool err = iAlarmInventory->removeAlarm( aAlarmEventID );
    if ( err == false ) {
        LOG_WARNING("No alarm found for ID " << aAlarmEventID);
    }
    else
    {
        LOG_DEBUG("Removed alarm, ID =" << aAlarmEventID);
    }
}


/*!
    \fn SyncScheduler::removeAllAlarms()
 */
void SyncScheduler::removeAllAlarms()
{
    FUNCTION_CALL_TRACE;
    
    QList<int> alarmList = iSyncScheduleProfiles.values();
    
    foreach (int alarmID, alarmList) {
        removeAlarmEvent(alarmID);
    }
}


/*!
    \fn SyncScheduler::deleteAlarmEvent(alarm_event_t* aEvent)
 */
/*
void SyncScheduler::deleteAlarmEvent(alarm_event_t* aEvent)
{
    alarm_event_delete(aEvent);
    aEvent = NULL;
}
*/


/*!
    \fn SyncScheduler::createAlarmEvent()
 */
/*
alarm_event_t* SyncScheduler::createAlarmEvent()
{
    return alarm_event_create();
}
*/
