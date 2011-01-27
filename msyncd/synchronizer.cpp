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
#include "synchronizer.h"
#include "SyncDBusAdaptor.h"
#include "SyncSession.h"
#include "ClientPluginRunner.h"
#include "ServerPluginRunner.h"
#include "AccountsHelper.h"
#include "TransportTracker.h"
#include "ServerActivator.h"

#include "SyncCommonDefs.h"
#include "StoragePlugin.h"
#include "SyncLog.h"
#include "ClientPlugin.h"
#include "ServerPlugin.h"
#include "ProfileFactory.h"
#include "ProfileEngineDefs.h"
#include "LogMacros.h"
#include "SyncDBusConnection.h"

#include <QtDebug>
#include <fcntl.h>
#include <termios.h>

using namespace Buteo;

static const QString SYNC_DBUS_OBJECT = "/synchronizer";
static const QString SYNC_DBUS_SERVICE = "com.meego.msyncd";

// Maximum time in milliseconds to wait for a thread to stop
static const unsigned long long MAX_THREAD_STOP_WAIT_TIME = 5000;

Synchronizer::Synchronizer( QCoreApplication* aApplication )
:   iSyncScheduler(0),
    iSyncBackup(0),    
    iTransportTracker(0),
    iServerActivator(0),
    iAccounts(0),
    iClosing(false),
    iSyncUIInterface(NULL)
{
    FUNCTION_CALL_TRACE;

    this->setParent(aApplication);
}

Synchronizer::~Synchronizer()
{
    FUNCTION_CALL_TRACE;
    //Clearing syncUiinterface
    if (iSyncUIInterface) {
        delete iSyncUIInterface;
        iSyncUIInterface = NULL;
    }
}

bool Synchronizer::initialize()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Starting msyncd");

    // Create a D-Bus adaptor. It will get deleted when the Synchronizer is
    // deleted.
    new SyncDBusAdaptor(this);

    // Register our object on the session bus and expose interface to others.
    QDBusConnection dbus = SyncDBusConnection::sessionBus();
    if (!dbus.registerObject(SYNC_DBUS_OBJECT, this) ||
            !dbus.registerService(SYNC_DBUS_SERVICE))
    {
        LOG_CRITICAL( "Failed to register to D-Bus (D-Bus not started or msyncd already running?), aborting start" );
        return false;
    }
    else
    {
        LOG_DEBUG("Registered to D-Bus");
    } // else ok

    connect(&iProfileManager,SIGNAL(signalProfileChanged(QString,int,QString)),
            this ,SIGNAL(signalProfileChanged(QString,int,QString)));

    iTransportTracker = new TransportTracker(this);

    if (iTransportTracker != 0)
    {
        iServerActivator = new ServerActivator(iProfileManager,
                *iTransportTracker, this);
    }

    // Initialize account manager.
    iAccounts = new AccountsHelper(iProfileManager, this); // Deleted with parent.

    connect(this, SIGNAL(storageReleased()),
            this, SLOT(onStorageReleased()), Qt::QueuedConnection);

    startServers();

    // Initialize scheduler
    initializeScheduler();

    // For Backup/restore handling
    iSyncBackup =  new SyncBackup();
    connect(iSyncBackup, SIGNAL(startBackup()),this, SLOT(backupStarts()));
    connect(iSyncBackup, SIGNAL(backupDone()),this, SLOT(backupFinished()));
    connect(iSyncBackup, SIGNAL(startRestore()),this, SLOT(restoreStarts()));
    connect(iSyncBackup, SIGNAL(restoreDone()),this, SLOT(restoreFinished()));

    //For Sync On Change
    QObject::connect(&iSyncOnChangeScheduler, SIGNAL(syncNow(QString)),
                     this, SLOT(startSync(QString)),
                     Qt::QueuedConnection);
    // enable SOC for contacts only as of now
    QHash<QString,QList<SyncProfile*> > aSOCStorageMap;
    //TODO can we do away with hard coding storage (plug-in) names, in other words do they
    //have to be well known this way
    QList<SyncProfile*> SOCProfiles = iProfileManager.getSOCProfilesForStorage("hcontacts");
    if(SOCProfiles.count())
    {
        aSOCStorageMap["hcontacts"] = SOCProfiles;
        foreach(const SyncProfile* aProfile, SOCProfiles)
        {
            LOG_DEBUG("Profile" << aProfile->name() <<"interested in SOC for contacts");
        }
        QStringList aFailedStorages;
        bool isSOCEnabled = iSyncOnChange.enable(aSOCStorageMap, &iSyncOnChangeScheduler,
                                                 &iPluginManager, aFailedStorages);
        if(!isSOCEnabled)
        {
            foreach(const QString& aStorageName, aFailedStorages)
            {
                LOG_WARNING("Sync on change couldn't be enabled for storage" << aStorageName);
            }
        }
    }
    return true;
}

void Synchronizer::close()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Stopping msyncd");

    iClosing = true;

    // Stop running sessions
    iSyncOnChange.disable();

    QList<SyncSession*> sessions = iActiveSessions.values();
    foreach (SyncSession* session, sessions)
    {
        if (session != 0)
        {
            session->stop();
        }
    }
    qDeleteAll(sessions);
    iActiveSessions.clear();

    stopServers();

    delete iSyncScheduler;
    iSyncScheduler = 0;

    delete iSyncBackup;
    iSyncBackup = 0;


    // Unregister from D-Bus.
    QDBusConnection dbus = SyncDBusConnection::sessionBus();
    dbus.unregisterObject(SYNC_DBUS_OBJECT);
    if (!dbus.unregisterService(SYNC_DBUS_SERVICE))
    {
        LOG_WARNING("Failed to unregister from D-Bus");
    }
    else
    {
        LOG_DEBUG("Unregistered from D-Bus");
    }

}

bool Synchronizer::startSync(QString aProfileName)
{
    FUNCTION_CALL_TRACE;

    // Manually triggered sync.
    return startSync(aProfileName, false);
}

bool Synchronizer::startScheduledSync(QString aProfileName)
{
    FUNCTION_CALL_TRACE;

    // Scheduled sync.
    return startSync(aProfileName, true);
}

bool Synchronizer::setSyncSchedule(QString aProfileId , QString aScheduleAsXml)
{
    bool status = false;
    if(iProfileManager.setSyncSchedule(aProfileId , aScheduleAsXml)) {
        reschedule(aProfileId);
        status = true;
    }
    return status;
}

bool Synchronizer:: saveSyncResults(QString aProfileId, QString aSyncResults)
{
    QDomDocument doc;
    bool status = false;
    if (doc.setContent(aSyncResults, true)) {
        Buteo::SyncResults results(doc.documentElement());
        status = iProfileManager.saveSyncResults(aProfileId , results);
    } else {
        LOG_CRITICAL("Invalid Profile Xml Received from msyncd");
    }

    return status;
}

bool Synchronizer::startSync(const QString &aProfileName, bool aScheduled)
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    if (isBackupRestoreInProgress()) {
        SyncResults syncResults(QDateTime::currentDateTime(), SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::BACKUP_IN_PROGRESS);
        iProfileManager.saveSyncResults(aProfileName, syncResults);
        return success;
    }

    LOG_DEBUG( "Start sync requested for profile:" << aProfileName );

    if (iActiveSessions.contains(aProfileName))
    {
        LOG_DEBUG( "Sync already in progress" );
        return true;
    }
    else if (iSyncQueue.contains(aProfileName))
    {
        LOG_DEBUG( "Sync request already in queue" );
        emit syncStatus(aProfileName, Sync::SYNC_QUEUED, "", 0);
        return true;
    }

    SyncProfile *profile = iProfileManager.syncProfile(aProfileName);
    if (!profile)
    {
        LOG_WARNING( "Profile not found" );
        SyncResults syncResults(QDateTime::currentDateTime(), SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
        iProfileManager.saveSyncResults(aProfileName, syncResults);
        emit syncStatus(aProfileName, Sync::SYNC_ERROR, "Internal Error" , Buteo::SyncResults::INTERNAL_ERROR);
        return false;
    }

    SyncSession *session = new SyncSession(profile, this);
    if (session == 0)
    {
        LOG_WARNING( "Failed to create sync session object" );
        delete profile;
        profile = 0;
        SyncResults syncResults(QDateTime::currentDateTime(), SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
        iProfileManager.saveSyncResults(aProfileName, syncResults);
        emit syncStatus(aProfileName, Sync::SYNC_ERROR, "Internal Error" , Buteo::SyncResults::INTERNAL_ERROR);
        return false;
    }

    // If profile hasn't been used before, we set profileCreated to true
    // so that we save remote Id when we clean up after the sync.
    if (profile->lastResults()==0) {
        LOG_DEBUG( "Setting ProfileCreated true");
        session->setProfileCreated(true);
    }

    restoreProfileCounter(profile);
    if ( profile->syncCurrentAttempt() == 0 ) {
        session->setScheduled(aScheduled);
    }
    else {
        session->setScheduled(false);
    }

    // @todo: Complete profile with data from account manager.
    //iAccounts->addAccountData(*profile);

    QtMobility::QSystemDeviceInfo::BatteryStatus batteryStat = iDeviceInfo.batteryStatus();
    //LOG_DEBUG("Battery status:"<<batteryStat);

    if (!profile->isValid())
    {
        LOG_WARNING( "Profile is not valid" );
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
        emit syncStatus(aProfileName, Sync::SYNC_ERROR, "Internal Error", Buteo::SyncResults::INTERNAL_ERROR);
    }

    else if( aScheduled && (batteryStat != QtMobility::QSystemDeviceInfo::BatteryNormal) )
    {
        LOG_DEBUG( "Low power, scheduled sync aborted" );
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::LOW_BATTERY_POWER);
        emit syncStatus(aProfileName, Sync::SYNC_ERROR, "Low battery", Buteo::SyncResults::LOW_BATTERY_POWER);
    }
    else if (!session->reserveStorages(&iStorageBooker))
    {
        LOG_DEBUG( "Needed storage(s) already in use, queuing sync request" );
        iSyncQueue.enqueue(session);
        emit syncStatus(aProfileName, Sync::SYNC_QUEUED, "", 0);
        success = true;
    }
    else
    {
        // Sync can be started now.
        success = startSyncNow(session);
        if (success)
        {
            emit syncStatus(aProfileName, Sync::SYNC_STARTED, "", 0);
        }
        else
        {
            session->setFailureResult(SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
            emit syncStatus(aProfileName, Sync::SYNC_ERROR, "Internal Error", Buteo::SyncResults::INTERNAL_ERROR);
        }
    }

    if (!success)
    {
        cleanupSession(session, Sync::SYNC_ERROR );
    } // no else

    return success;
}

bool Synchronizer::startSyncNow(SyncSession *aSession)
{
    FUNCTION_CALL_TRACE;

    if (!aSession || isBackupRestoreInProgress())
    {
        LOG_WARNING( "Session is null || backup in progress" );
        return false;
    }

    SyncProfile *profile = aSession->profile();
    if (!profile)
    {
        LOG_WARNING( "Profile in session is null" );
        return false;
    }

    LOG_DEBUG( "Starting sync with profile" <<  aSession->profileName());

    Profile *clientProfile = profile->clientProfile();
    if (clientProfile == 0) {
        LOG_WARNING( "Could not find client sub-profile" );
        return false;
    }
    PluginRunner *pluginRunner = new ClientPluginRunner(
            clientProfile->name(), aSession->profile(), &iPluginManager, this,
            this);
    aSession->setPluginRunner(pluginRunner, true);
    if (pluginRunner == 0 || !pluginRunner->init())
    {
        LOG_WARNING( "Failed to initialize client plug-in runner" );
        return false;
    }

    // Relay connectivity state change signal to plug-in runner.
    connect(iTransportTracker, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)),
            pluginRunner, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)));

    LOG_DEBUG( "Client plug-in runner initialized" );

    // Connect signals from sync session.
    connect(aSession, SIGNAL(transferProgress(const QString &,
            Sync::TransferDatabase, Sync::TransferType, const QString &, int)),
            this, SLOT(onTransferProgress(const QString &,
                    Sync::TransferDatabase, Sync::TransferType, const QString &, int)));

    connect(aSession, SIGNAL(storageAccquired(const QString &, const QString &)),
            this, SLOT(onStorageAccquired(const QString &, const QString &)));
    connect(aSession, SIGNAL(syncProgressDetail(const QString &,int)),
            this, SLOT(onSyncProgressDetail(const QString &,int)));

    connect(aSession, SIGNAL(finished(const QString &, Sync::SyncStatus,
            const QString &, int)),
            this, SLOT(onSessionFinished(const QString &, Sync::SyncStatus,
                    const QString &, int)));

    if (aSession->start())
    {
        // Get the DBUS interface for sync-UI.
        LOG_DEBUG( "sync-ui dbus interface is getting called" );
        if (aSession->isScheduled()) {
            if (iSyncUIInterface == NULL) {
                LOG_DEBUG( "iSyncUIInterface is Null" );
                iSyncUIInterface = new QDBusInterface("com.nokia.syncui", "/org/maemo/m",
                        "com.nokia.MApplicationIf", SyncDBusConnection::sessionBus() );
                Q_ASSERT(iSyncUIInterface);
            }
            else if(!iSyncUIInterface->isValid()) {
                LOG_DEBUG( "iSyncUIInterface is not valid" );
                delete iSyncUIInterface;
                iSyncUIInterface = NULL;
                iSyncUIInterface = new QDBusInterface("com.nokia.syncui", "/org/maemo/m",
                        "com.nokia.MApplicationIf", SyncDBusConnection::sessionBus() );
                Q_ASSERT(iSyncUIInterface);
            }
            //calling launch with argument list
            QStringList list;
            list.append("launching");
            QList<QVariant> argumentList;
            argumentList << qVariantFromValue(list);
            iSyncUIInterface->asyncCallWithArgumentList(QLatin1String("launch"), argumentList);
        }

        LOG_DEBUG( "Sync session started" );
        iActiveSessions.insert(aSession->profileName(), aSession);
    }
    else
    {
        LOG_WARNING( "Failed to start sync session" );
        return false;
    }

    return true;
}

void Synchronizer::onSessionFinished(const QString &aProfileName,
        Sync::SyncStatus aStatus, const QString &aMessage, int aErrorCode)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Session finished:" << aProfileName << ", status:" << aStatus);

    if(iActiveSessions.contains(aProfileName))
    {
        SyncSession *session = iActiveSessions[aProfileName];
        if (session)
        {
            switch(aStatus)
            {
            case Sync::SYNC_DONE:
            {
                QMap<QString,bool> storageMap = session->getStorageMap();
                //session->setFailureResult(SyncResults::SYNC_RESULT_SUCCESS, Buteo::SyncResults::NO_ERROR);
                SyncProfile *sessionProf = session->profile();
                iProfileManager.enableStorages(*sessionProf, storageMap);
                
                // If caps have not been modified, i.e. fetched from the remote device yet, set 
                // enabled storages also visible. If caps have been modified, we must not touch visibility anymore
                if (sessionProf->boolKey(KEY_CAPS_MODIFIED) == false)
                {
                    iProfileManager.setStoragesVisible(*sessionProf, storageMap);
                }
                
                iProfileManager.updateProfile(*sessionProf);
                sessionProf->resetAttempts();
                break;
            }

            case Sync::SYNC_ABORTED:
            case Sync::SYNC_CANCELLED:
            {
                session->setFailureResult(SyncResults::SYNC_RESULT_CANCELLED, Buteo::SyncResults::ABORTED);
                session->profile()->resetAttempts();
                break;
            }
            case Sync::SYNC_ERROR:
            {
                session->setFailureResult(SyncResults::SYNC_RESULT_FAILED, aErrorCode);
                restoreProfileCounter(session->profile());
                session->profile()->setNextAttempt();
                saveProfileCounter(session->profile());
                if ( !session->profile()->needNextAttempt() )
                {
                    session->profile()->resetAttempts();
                    saveProfileCounter(session->profile());
                }
                break;
            }

            default:
                LOG_WARNING("Unhandled Status in onSessionFinished" << aStatus);
                break;
            }

            iActiveSessions.remove(aProfileName);
            cleanupSession(session, aStatus);
            if (session->isAborted() && (iActiveSessions.size() == 0) && isBackupRestoreInProgress()) {
                stopServers();
                iSyncBackup->sendReply(0);
            }
        } 
        else {
            LOG_WARNING( "Session found in active sessions, but is NULL" );
        }
    }
    else
    {
        LOG_WARNING( "Session not found from active sessions" );
    }

    emit syncStatus(aProfileName, aStatus, aMessage, aErrorCode);

    // Try starting new sync sessions waiting in the queue.
    while (startNextSync())
    {
        //intentionally empty
    }
}

void Synchronizer::onSyncProgressDetail(const QString &aProfileName,int aProgressDetail)
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("aProfileName"<<aProfileName);
    emit syncStatus(aProfileName, Sync::SYNC_PROGRESS, "Sync Progress", aProgressDetail);
}

bool Synchronizer::startNextSync()
{
    FUNCTION_CALL_TRACE;

    if (iSyncQueue.isEmpty() || isBackupRestoreInProgress()) {
        return false;
    }        

    bool tryNext = true;

    SyncSession *session = iSyncQueue.head();
    if (session == 0)
    {
        LOG_WARNING( "Null session found from queue" );
        iSyncQueue.dequeue();
        return true;
    }

    SyncProfile *profile = session->profile();
    if (profile == 0)
    {
        LOG_WARNING( "Null profile found from queued session" );
        cleanupSession(session, Sync::SYNC_ERROR);
        iSyncQueue.dequeue();
        return true;
    }

    QString profileName = session->profileName();
    LOG_DEBUG( "Trying to start next sync in queue. Profile:" << profileName );

    QtMobility::QSystemDeviceInfo::BatteryStatus batteryStat = iDeviceInfo.batteryStatus();

    if (session->isScheduled() && (batteryStat != QtMobility::QSystemDeviceInfo::BatteryNormal))
    {
        LOG_DEBUG( "Low power, scheduled sync aborted" );
        iSyncQueue.dequeue();
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::LOW_BATTERY_POWER);
        cleanupSession(session, Sync::SYNC_ERROR);
        emit syncStatus(profileName, Sync::SYNC_ERROR, "Low Battery", Buteo::SyncResults::LOW_BATTERY_POWER);
        tryNext = true;
    }
    else if (!session->reserveStorages(&iStorageBooker))
    {
        LOG_DEBUG( "Needed storage(s) already in use" );
        tryNext = false;
    }
    else
    {
        // Sync can be started now.
        iSyncQueue.dequeue();
        if (startSyncNow(session))
        {
            emit syncStatus(profileName, Sync::SYNC_STARTED, "", 0);
        }
        else
        {
            session->setFailureResult(SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
            cleanupSession(session, Sync::SYNC_ERROR);
            emit syncStatus(profileName, Sync::SYNC_ERROR, "Internal Error", Buteo::SyncResults::INTERNAL_ERROR);
        }
        tryNext = true;
    }

    return tryNext;
}

void Synchronizer::cleanupSession(SyncSession *aSession, Sync::SyncStatus aStatus)
{
    FUNCTION_CALL_TRACE;

    if (aSession != 0)
    {
        QString profileName = aSession->profileName();
        if (!profileName.isEmpty())
        {
            LOG_DEBUG("aStatus"<<aStatus);
            if (aSession->isProfileCreated() && (aStatus == Sync::SYNC_DONE)) {
                iProfileManager.saveRemoteTargetId(*aSession->profile(), aSession->results().getTargetId());
            }
            iProfileManager.saveSyncResults(profileName, aSession->results());

            // UI needs to know that Sync Log has been updated.
            emit resultsAvailable(profileName,aSession->results().toString());

            if ( aSession->isScheduled() || aSession->profile()->needNextAttempt() )
            {
                reschedule(profileName);
                emit signalProfileChanged(profileName, 1 ,QString());
            } // no else
        } // no else
        aSession->setProfileCreated(false);
        aSession->releaseStorages();
        aSession->deleteLater();
        aSession = 0;
    }
}

void Synchronizer::abortSync(QString aProfileName)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Abort sync requested for profile: " << aProfileName );
    if (iActiveSessions.contains(aProfileName))
    {
        iActiveSessions[aProfileName]->abort();
    }
    else
    {
        LOG_WARNING( "No sync in progress with the given profile" );
        SyncResults syncResults(QDateTime::currentDateTime(), SyncResults::SYNC_RESULT_CANCELLED, Buteo::SyncResults::ABORTED);
        iProfileManager.saveSyncResults(aProfileName, syncResults);
        emit syncStatus(aProfileName, Sync::SYNC_CANCELLED, "", Buteo::SyncResults::ABORTED);
    }
}



bool Synchronizer::removeProfile(QString aProfileId)
{
    FUNCTION_CALL_TRACE
    bool status = false;
    // We assume this call is made on a Sync Profile
    SyncProfile *profile = iProfileManager.syncProfile (aProfileId);

    if(!aProfileId.isEmpty() && profile)  {

        bool client = true ;
        Profile *subProfile = profile->clientProfile(); // client or server
        if ( !subProfile) {
            LOG_WARNING( "Could not find client sub-profile" );
            subProfile = profile->serverProfile ();
            client = false;
            if (!subProfile){
                LOG_WARNING( "Could not find server sub-profile" );
                return status;
            }
        }

        if (profile->syncType() == SyncProfile::SYNC_SCHEDULED){
            iSyncScheduler->removeProfile(aProfileId);
        }

        PluginRunner *pluginRunner;
        if (client) {
            pluginRunner = new ClientPluginRunner(subProfile->name(), profile, &iPluginManager, this, this);
        } else {
            pluginRunner = new ServerPluginRunner(subProfile->name(), profile, &iPluginManager, this,
                    iServerActivator, this);
        }

        if (pluginRunner == 0 || !pluginRunner->init())
        {
            LOG_WARNING( "Failed to initialize client plug-in runner" );
            delete profile;
            return status;
        }

        const SyncResults * syncResults = profile->lastResults();
        if (!pluginRunner->cleanUp() && syncResults){
            LOG_CRITICAL ("Error in removing anchors, sync session ");
        } else {
            LOG_DEBUG("Removing the profile");
            iProfileManager.removeProfile(aProfileId);
            status = true;
        }
        delete profile;
        delete pluginRunner;
    }
    return status;
}

bool Synchronizer::updateProfile(QString aProfileAsXml)
{
    FUNCTION_CALL_TRACE
    bool status = false;
    if(!aProfileAsXml.isEmpty())  {
        // save the changes to persistent storage
        Profile *profile = iProfileManager.profileFromXml(aProfileAsXml);
        if(profile) {
            QString profileId = iProfileManager.updateProfile(*profile);

            // if the profile changes are for schedule sync we need to reschedule
            if(!profileId.isEmpty()) {
                reschedule(profileId);
                status = true;
            }

            delete profile;
        }
    }
    return status;
}

bool Synchronizer::requestStorages(QStringList aStorageNames)
{
    FUNCTION_CALL_TRACE;

    return iStorageBooker.reserveStorages(aStorageNames, "");
}

void Synchronizer::releaseStorages(QStringList aStorageNames)
{
    FUNCTION_CALL_TRACE;

    iStorageBooker.releaseStorages(aStorageNames);
    emit storageReleased();
}

QStringList Synchronizer::runningSyncs()
{
    FUNCTION_CALL_TRACE;

    return iActiveSessions.keys();
}

void Synchronizer::onStorageReleased()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Storage released" );
    while (startNextSync())
    {
        // Intentionally empty.
    }
}

void Synchronizer::onTransferProgress( const QString &aProfileName,
        Sync::TransferDatabase aDatabase, Sync::TransferType aType,
        const QString &aMimeType, int aCommittedItems )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Sync session progress" );
    LOG_DEBUG( "Profile:" << aProfileName );
    LOG_DEBUG( "Database:" << aDatabase );
    LOG_DEBUG( "Transfer type:" << aType );
    LOG_DEBUG( "Mime type:" << aMimeType );

    emit transferProgress( aProfileName, aDatabase, aType, aMimeType, aCommittedItems );

}

void Synchronizer::onStorageAccquired ( const QString &aProfileName, 
        const QString &aMimeType )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG( "Mime type:" << aMimeType );
    LOG_DEBUG( "Profile:" << aProfileName );
    if (!aProfileName.isEmpty() && !aMimeType.isEmpty()){
        SyncSession *session = qobject_cast<SyncSession*>(QObject::sender());
        if (session){
            QMap<QString,bool> storageMap = session->getStorageMap();
            if (aMimeType.compare(QString("text/x-vcard"), Qt::CaseInsensitive) == 0)
                storageMap["hcontacts"] = true;
            else if (aMimeType.compare(QString("text/x-vcalendar"), Qt::CaseInsensitive) == 0)
                storageMap["hcalendar"] = true;
            else if (aMimeType.compare(QString("text/plain"), Qt::CaseInsensitive) == 0)
                storageMap["hnotes"] = true;
            else if (aMimeType.compare(QString("text/x-vbookmark"), Qt::CaseInsensitive) == 0)
                storageMap["hbookmarks"] = true;
            else if (aMimeType.compare(QString("text/x-vmsg"), Qt::CaseInsensitive) == 0)
                storageMap["hsms"] = true;
            else 
                LOG_DEBUG( "Unsupported mime type" << aMimeType );
            
            session->setStorageMap(storageMap);
        }
    }
}

bool Synchronizer::requestStorage(const QString &aStorageName,
        const SyncPluginBase *aCaller)
{
    FUNCTION_CALL_TRACE;

    return iStorageBooker.reserveStorage(aStorageName,
            aCaller->getProfileName());
}

void Synchronizer::releaseStorage(const QString &aStorageName,
        const SyncPluginBase */*aCaller*/)
{
    FUNCTION_CALL_TRACE;

    iStorageBooker.releaseStorage(aStorageName);
    emit storageReleased();
}

StoragePlugin* Synchronizer::createStorage(const QString &aPluginName)
{
    FUNCTION_CALL_TRACE;

    StoragePlugin* plugin = NULL;
    if (!aPluginName.isEmpty())
    {
        plugin = iPluginManager.createStorage(aPluginName);
    } // no else

    return plugin;
}

void Synchronizer::initializeScheduler()
{
    FUNCTION_CALL_TRACE;
    if (!iSyncScheduler) {
        iSyncScheduler = new SyncScheduler(this);
        connect(iSyncScheduler, SIGNAL(syncNow(QString)),
                this, SLOT(startScheduledSync(QString)), Qt::QueuedConnection);
        QList<SyncProfile*> profiles = iProfileManager.allSyncProfiles();
        foreach (SyncProfile *profile, profiles)
        {
            if (profile->syncType() == SyncProfile::SYNC_SCHEDULED)
            {
                iSyncScheduler->addProfile(profile);
            } // no else
        }
        qDeleteAll(profiles);
    }
}

void Synchronizer::destroyStorage(StoragePlugin *aStorage)
{
    FUNCTION_CALL_TRACE;

    iPluginManager.destroyStorage(aStorage);
}

bool Synchronizer::isConnectivityAvailable( Sync::ConnectivityType aType )
{
    FUNCTION_CALL_TRACE;

    if (iTransportTracker != 0)
    {
        return iTransportTracker->isConnectivityAvailable(aType);
    }
    else
    {
        return false;
    }
}

void Synchronizer::startServers( bool resume )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Starting/Resuming server plug-ins" );

    if (iServerActivator != 0)
    {
        if( false == resume )
        {
            connect(iServerActivator, SIGNAL(serverEnabled(const QString &)),
                    this, SLOT(startServer(const QString &)), Qt::QueuedConnection);

            connect(iServerActivator, SIGNAL(serverDisabled(const QString &)),
                    this, SLOT(stopServer(const QString &)), Qt::QueuedConnection);
        }

        QStringList enabledServers = iServerActivator->enabledServers();
        foreach (QString server, enabledServers)
        {
            if( false == resume )
            {
                startServer(server);
            }
            else
            {
                ServerPluginRunner *pluginRunner = iServers[server];
                if( pluginRunner )
                {
                    pluginRunner->resume();
                }
            }
        }
    }
    else
    {
        LOG_CRITICAL("No server plug-in activator");
    }
}

void Synchronizer::stopServers( bool suspend )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Stopping/Suspending all server plug-ins" );

    if( false == suspend )
    {
        iServerActivator->disconnect();
    }

    QStringList activeServers = iServers.keys();
    foreach (QString server, activeServers)
    {
        if( false == suspend )
        {
            stopServer(server);
        }
        else
        {
            ServerPluginRunner *pluginRunner = iServers[server];
            if( pluginRunner )
            {
                pluginRunner->suspend();
            }
        }
    }
}

void Synchronizer::startServer(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Starting server plug-in:" << aProfileName);

    if(iServers.contains(aProfileName))
    {
        LOG_WARNING("Server thread already running for profile:" << aProfileName);
        // Remove reference from the activator
        iServerActivator->removeRef(aProfileName, false);
        return;
    }
    Profile* serverProfile = iProfileManager.profile(
            aProfileName, Profile::TYPE_SERVER );

    if (!serverProfile)
    {
        // @todo: for now, do not enforce server plug-ins to have an XML profile
        LOG_WARNING( "Profile not found, creating an empty one" );

        ProfileFactory pf;
        serverProfile = pf.createProfile(aProfileName, Profile::TYPE_SERVER);
    }
    else
    {
        iProfileManager.expand( *serverProfile );
    }

    if( !serverProfile || !serverProfile->isValid() ) {
        LOG_WARNING( "Profile not found or not valid:"  << aProfileName );
        delete serverProfile;
        serverProfile = 0;
        return;
    }

    ServerPluginRunner *pluginRunner = new ServerPluginRunner(aProfileName,
            serverProfile, &iPluginManager, this, iServerActivator, this);
    if (pluginRunner == 0)
    {
        LOG_CRITICAL("Failed to create plug-in runner");
        delete serverProfile;
        serverProfile = 0;
        return;
    }

    // Relay connectivity state change signal to plug-in runner.
    connect(iTransportTracker, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)),
            pluginRunner, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)));

    connect(pluginRunner, SIGNAL(done()), this, SLOT(onServerDone()));

    connect(pluginRunner, SIGNAL(newSession(const QString &)),
            this, SLOT(onNewSession(const QString &)));

    if (!pluginRunner->init() || !pluginRunner->start())
    {
        LOG_CRITICAL("Failed to start plug-in");
        delete pluginRunner;
        pluginRunner = 0;
        return;
    }

    iServers.insert(aProfileName, pluginRunner);

}

void Synchronizer::stopServer( const QString& aProfileName )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Stopping server:" << aProfileName);

    if (iServers.contains(aProfileName))
    {
        ServerPluginRunner *pluginRunner = iServers[aProfileName];
        pluginRunner->stop();
        LOG_DEBUG("Deleting server");
        if (!iClosing)
        {
            // This function may have been invoked from a signal. The plugin runner
            // will only be deleted when the server thread returns.
            LOG_WARNING("The server thread for profile: " << aProfileName <<
                    "is still running. Server will be deleted later");
        }
        else
        {
            iServers.remove(aProfileName);
            // Synchronizer is closing, this function is not invoked by a signal.
            // Delete plug-in runner immediately.
            delete pluginRunner;
        }
        pluginRunner = 0;
    }
    else
    {
        LOG_WARNING("Server not found");
    }
}

void Synchronizer::onServerDone()
{
    FUNCTION_CALL_TRACE;

    ServerPluginRunner *pluginRunner =
            qobject_cast<ServerPluginRunner*>(QObject::sender());
    QString serverName = "unknown";
    if (pluginRunner != 0)
    {
        serverName = pluginRunner->pluginName();
    }
    LOG_DEBUG("Server stopped:" << serverName);
    if (iServers.values().contains(pluginRunner))
    {
        LOG_DEBUG("Deleting server");
        iServers.remove(iServers.key(pluginRunner));
        pluginRunner->deleteLater();
        pluginRunner = 0;
    }
}

bool syncProfilePointerLessThan(SyncProfile *&aLhs, SyncProfile *&aRhs)
{
    if (aLhs && aRhs) {
        if (aLhs->isHidden() != aRhs->isHidden())
            return !aLhs->isHidden();
        if (aLhs->isEnabled() != aRhs->isEnabled())
            return aLhs->isEnabled();
    }

    return false;
}

void Synchronizer::onNewSession(const QString &aDestination)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("New session from" << aDestination);
    bool createNewProfile = false;
    ServerPluginRunner *pluginRunner =
            qobject_cast<ServerPluginRunner*>(QObject::sender());
    if (pluginRunner != 0)
    {
        SyncProfile *profile = 0;
        QList<SyncProfile*> syncProfiles;
        if (!aDestination.contains("USB")) // Search profile only with BT
        {
            syncProfiles = iProfileManager.getSyncProfilesByData(
                    QString::null, Profile::TYPE_SERVICE, KEY_BT_ADDRESS, aDestination);
        } // no else
        if (syncProfiles.isEmpty())
        {
            // destination a bt address
            profile = iProfileManager.createTempSyncProfile(aDestination, createNewProfile);
            LOG_DEBUG( "No sync profiles found with a matching destination address" );
        }
        else
        {
            // Sort profiles to preference order. Visible and enabled are preferred.
            qSort(syncProfiles.begin(), syncProfiles.end(), syncProfilePointerLessThan);

            profile = syncProfiles.first();
            LOG_DEBUG( "Found" << syncProfiles.count() << "sync profiles with a "
                    "matching destination address. Selecting" <<
                    profile->name());
            syncProfiles.removeFirst();
            qDeleteAll(syncProfiles);
        }

    // If the profile is not hidden, UI must be informed.
	if(!profile->isHidden())
	{
	    // Get the DBUS interface for sync-UI.
	    LOG_DEBUG( "sync-ui dbus interface is getting called" );
	    if (iSyncUIInterface == NULL) {
		LOG_DEBUG( "iSyncUIInterface is NULL" );
		iSyncUIInterface = new QDBusInterface("com.nokia.syncui", "/org/maemo/m",
			"com.nokia.MApplicationIf", SyncDBusConnection::sessionBus() );
		Q_ASSERT(iSyncUIInterface);
	    }
	    else if(!iSyncUIInterface->isValid()) {
		LOG_DEBUG( "iSyncUIInterface is not Valid()" );
		delete iSyncUIInterface;
		iSyncUIInterface = NULL;
		iSyncUIInterface = new QDBusInterface("com.nokia.syncui", "/org/maemo/m",
			"com.nokia.MApplicationIf", SyncDBusConnection::sessionBus() );
		Q_ASSERT(iSyncUIInterface);

	    }
	    //calling launch with argument list
	    QStringList list;
	    list.append("launching");
	    QList<QVariant> argumentList;
	    argumentList << qVariantFromValue(list);
	    iSyncUIInterface->asyncCallWithArgumentList(QLatin1String("launch"), argumentList);
	}

        SyncSession *session = new SyncSession(profile, this);
        if (session != 0)
        {
            session->setProfileCreated(createNewProfile);
            // disable all storages
            // @todo : Can we remove hardcoding of the storageNames ???
            QMap<QString,bool> storageMap;
            storageMap["hcontacts"] = 0;
            storageMap["hcalendar"] = 0;
            storageMap["hnotes"] = 0;
            storageMap["hbookmarks"] = 0;
            storageMap["hsms"] = 0;

            session->setStorageMap(storageMap);

            iActiveSessions.insert(profile->name(), session);

            // Connect signals from sync session.
            connect(session, SIGNAL(transferProgress(const QString &,
                    Sync::TransferDatabase, Sync::TransferType, const QString &, int)),
                    this, SLOT(onTransferProgress(const QString &,
                            Sync::TransferDatabase, Sync::TransferType, const QString &, int)));
            connect(session, SIGNAL(storageAccquired(const QString &, const QString &)),
                    this, SLOT(onStorageAccquired(const QString &, const QString &)));
            connect(session, SIGNAL(finished(const QString &, Sync::SyncStatus,
                    const QString &, int)),
                    this, SLOT(onSessionFinished(const QString &, Sync::SyncStatus,
                            const QString &, int)));
            connect(session, SIGNAL(syncProgressDetail(const QString &,int)),
                    this, SLOT(onSyncProgressDetail(const QString &,int)));

            // Associate plug-in runner with the new session.
            session->setPluginRunner(pluginRunner, false);
            emit syncStatus(profile->name(), Sync::SYNC_STARTED, "", 0);
        }
        else
        {
            delete profile;
            profile = 0;
            LOG_CRITICAL("Failed to create session object");
        }
    }
    else
    {
        LOG_WARNING("Could not resolve server, session object not created");
    }
}

void Synchronizer::reschedule(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE;

    if (iSyncScheduler == 0)
        return;

    SyncProfile *profile = iProfileManager.syncProfile(aProfileName);

    restoreProfileCounter(profile);
    if (profile && ((profile->syncType() == SyncProfile::SYNC_SCHEDULED) || (profile->needNextAttempt())))
    {
        iSyncScheduler->addProfile(profile);
    }
    else
    {
        iSyncScheduler->removeProfile(aProfileName);
    }
    if(profile)
    {
        delete profile;
        profile = NULL;
    }
}

bool Synchronizer::isBackupRestoreInProgress ()
{
    FUNCTION_CALL_TRACE;

    bool retVal = getBackUpRestoreState();

    if (retVal) {
        LOG_DEBUG ("Backup-Restore o/p in progress - Failed to start manual sync");
    }

    return retVal;

}

void Synchronizer::backupRestoreStarts ()
{
    LOG_DEBUG ("Synchronizer:backupRestoreStarts:");

    iClosing =  true;
    // No active sessions currently !!
    if (iActiveSessions.size() == 0) {
        LOG_DEBUG ("No active sync sessions ");
        stopServers( true );
        iSyncBackup->sendReply(0);
    } else {
        // Stop running sessions
        QList<SyncSession*> sessions = iActiveSessions.values();
        foreach (SyncSession* session, sessions)
        {
            if (session != 0)
            {
                session->abort();
            }
        }
    }

    delete iSyncScheduler;
    iSyncScheduler = 0;

}

void Synchronizer::backupRestoreFinished()
{
    LOG_DEBUG ("Synchronizer::backupFinished");
    iClosing =  false;
    startServers( true );
    initializeScheduler();
    iSyncBackup->sendReply(0);
}

void Synchronizer::backupStarts()
{
    LOG_DEBUG ("Synchronizer::backupStarts");
    emit backupInProgress();
    backupRestoreStarts ();
}

void Synchronizer::backupFinished()
{
    LOG_DEBUG ("Synchronizer::backupFinished");
    backupRestoreFinished();
    emit backupDone();
}

void Synchronizer::restoreStarts()
{
    LOG_DEBUG ("Synchronizer::restoreStarts");
    emit restoreInProgress();
    backupRestoreStarts();
}

void Synchronizer::restoreFinished()
{
    LOG_DEBUG ("Synchronizer::restoreFinished");
    backupRestoreFinished();
    emit restoreDone();
}

bool Synchronizer::getBackUpRestoreState()
{
    LOG_DEBUG ("Synchronizer::getBackUpRestoreState");
    return iSyncBackup->getBackUpRestoreState();
}

QString Synchronizer::getLastSyncResult(const QString &aProfileId)
{
    FUNCTION_CALL_TRACE;
    QString lastSyncResult;

    if(!aProfileId.isEmpty()) {
        SyncProfile *profile = iProfileManager.syncProfile (aProfileId);
        if(profile) {
            const SyncResults * syncResults = profile->lastResults();
            if (syncResults) {
                lastSyncResult = syncResults->toString();
                LOG_DEBUG("SyncResults found:"<<lastSyncResult);
            }
            else {
                LOG_DEBUG("SyncResults not Found!!!");
            }
            delete profile;
        }
        else {

            LOG_DEBUG("No profile found with aProfileId"<<aProfileId);
        }
    }
    return lastSyncResult;
}

QStringList Synchronizer::allVisibleSyncProfiles()
{
    FUNCTION_CALL_TRACE;
    QStringList profilesAsXml;

    QList<Buteo::SyncProfile *> profiles = iProfileManager.allVisibleSyncProfiles();

    if(!profiles.isEmpty()) {
        foreach(Buteo::SyncProfile *profile , profiles) {
            if(profile) {
                profilesAsXml.append(profile->toString());
                delete profile;
                profile = NULL;
            }
        }
    }
    LOG_DEBUG("allVisibleSyncProfiles profilesAsXml"<<profilesAsXml);
    return profilesAsXml;
}


QString Synchronizer::syncProfile(const QString &aProfileId)
{
    FUNCTION_CALL_TRACE;
    QString profileAsXml;

    if(!aProfileId.isEmpty()) {
        SyncProfile *profile = iProfileManager.syncProfile (aProfileId);
        if(profile) {
            profileAsXml.append(profile->toString());
            delete profile;
            profile = NULL;
        }
        else {

            LOG_DEBUG("No profile found with aProfileId"<<aProfileId);
        }
    }
    LOG_DEBUG("syncProfile profileAsXml"<<profileAsXml<<"aProfileId"<<aProfileId);
    return profileAsXml;
}

QStringList Synchronizer::syncProfilesByKey(const QString &aKey, const QString &aValue)
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("syncProfile key : "<< aKey <<"Value :"<< aValue);
    QStringList profilesAsXml;

    if(!aKey.isEmpty() && !aValue.isEmpty()) {
        QList<ProfileManager::SearchCriteria> filters;
        ProfileManager::SearchCriteria filter;
        filter.iType = ProfileManager::SearchCriteria::EQUAL;
        filter.iKey = aKey;
        filter.iValue = aValue;
        filters.append(filter);
	QList<SyncProfile*> profiles = iProfileManager.getSyncProfilesByData(filters);

	if (profiles.size() > 0) {
	    LOG_DEBUG("Found matching profiles  :" << profiles.size());	
            foreach (SyncProfile *profile, profiles) {
               profilesAsXml.append(profile->toString());
	    }
            qDeleteAll(profiles);
	} else {
            LOG_DEBUG("No profile found with key :" << aKey << "Value : " << aValue );
        }
    }
    
    return profilesAsXml;
}

QStringList Synchronizer::syncProfilesByType(const QString &aType)
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Profile Type : "<< aType);
    return iProfileManager.profileNames(aType);     
}

void Synchronizer::saveProfileCounter(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    int current = aProfile->syncCurrentAttempt();
    iCountersStorage[aProfile->name()] = current;
}

void Synchronizer::restoreProfileCounter(SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    if (aProfile) {
        if ( iCountersStorage.contains(aProfile->name()) )  {
            aProfile->setSyncRetryAttemp(iCountersStorage.value(aProfile->name()));
        }
        else {
            aProfile->setSyncRetryAttemp(0);
        }
    }
}

