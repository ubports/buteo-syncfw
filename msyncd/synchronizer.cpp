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

#include <contextsubscriber/contextproperty.h>
#include <QtDebug>
#include <fcntl.h>
#include <termios.h>

using namespace Buteo;

static const QString SYNC_DBUS_OBJECT = "/synchronizer";
static const QString SYNC_DBUS_SERVICE = "com.nokia.msyncd";

// Maximum time in milliseconds to wait for a thread to stop
static const unsigned long long MAX_THREAD_STOP_WAIT_TIME = 5000;

Synchronizer::Synchronizer( QCoreApplication* aApplication )
:   iSyncScheduler(0),
    iTransportTracker(0),
    iServerActivator(0),
    iAccounts(0),
    iClosing(false)
{
    FUNCTION_CALL_TRACE;

    this->setParent(aApplication);
}

Synchronizer::~Synchronizer()
{
    FUNCTION_CALL_TRACE;
}

bool Synchronizer::initialize()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Starting msyncd");

    // Create a D-Bus adaptor. It will get deleted when the Synchronizer is
    // deleted.
    new SyncDBusAdaptor(this);

    // Register our object on the session bus and expose interface to others.
    QDBusConnection dbus = QDBusConnection::sessionBus();
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

    iSyncScheduler = new SyncScheduler(this);

    iTransportTracker = new TransportTracker(this);

    if (iTransportTracker != 0)
    {
        iServerActivator = new ServerActivator(iProfileManager,
            *iTransportTracker, this);
    }

    // Initialize account manager.
    iAccounts = new AccountsHelper(iProfileManager, this); // Deleted with parent.

    QObject::connect(iAccounts, SIGNAL(profileChanged(QString,int)),
                     this, SIGNAL(signalProfileChanged(QString,int)));

    // Context property for low battery situation.
    iLowPower = new ContextProperty("Battery.LowBattery", this);

    connect(this, SIGNAL(storageReleased()),
        this, SLOT(onStorageReleased()), Qt::QueuedConnection);

    startServers();

    // Initialize scheduler
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

    return true;
}

void Synchronizer::close()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Stopping msyncd");

    iClosing = true;
 
    // Stop running sessions
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

    // Unregister from D-Bus.
    QDBusConnection dbus = QDBusConnection::sessionBus();
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

bool Synchronizer::startSync(const QString &aProfileName, bool aScheduled)
{
    FUNCTION_CALL_TRACE;

    bool success = false;

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
        return false;
    }

    SyncSession *session = new SyncSession(profile, this);
    if (session == 0)
    {
        LOG_WARNING( "Failed to create sync session object" );
        delete profile;
        profile = 0;
        return false;
    }
    session->setScheduled(aScheduled);

    // @todo: Complete profile with data from account manager.
    //iAccounts->addAccountData(*profile);

    if (!profile->isValid())
    {
        LOG_WARNING( "Profile is not valid" );
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
    }
    else if (aScheduled && iLowPower != 0 && iLowPower->value().toBool())
    {
        LOG_DEBUG( "Low power, scheduled sync aborted" );
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
    }
    else if (!isTransportAvailable(session))
    {
        LOG_DEBUG( "Required transport not available" );
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
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
            session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
        }
    }

    if (!success)
    {
        cleanupSession(session);
    } // no else

    return success;
}

bool Synchronizer::startSyncNow(SyncSession *aSession)
{
    FUNCTION_CALL_TRACE;

    if (!aSession)
    {
        LOG_WARNING( "Session is null" );
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
        Sync::TransferDatabase, Sync::TransferType, const QString &)),
        this, SLOT(onTransferProgress(const QString &,
        Sync::TransferDatabase, Sync::TransferType, const QString &)));
    connect(aSession, SIGNAL(storageAccquired(const QString &, const QString &)),
	this, SLOT(onStorageAccquired(const QString &, const QString &)));
    connect(aSession, SIGNAL(finished(const QString &, Sync::SyncStatus,
        const QString &, int)),
        this, SLOT(onSessionFinished(const QString &, Sync::SyncStatus,
        const QString &, int)));

    if (aSession->start())
    {
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
        if (session != 0)
        {
            if (session->iCreateProfile && (aStatus == Sync::SYNC_DONE)){   
                iProfileManager.enableStorages (*session->profile(), session->aStorageMap);
                iProfileManager.save(*session->profile());
            }
            iActiveSessions.remove(aProfileName);
            cleanupSession(session);
        }
        else
        {
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
        // Intentionally empty.
    }
}

bool Synchronizer::startNextSync()
{
    FUNCTION_CALL_TRACE;

    if (iSyncQueue.isEmpty())
        return false;

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
        cleanupSession(session);
        iSyncQueue.dequeue();
        return true;
    }

    QString profileName = session->profileName();
    LOG_DEBUG( "Trying to start next sync in queue. Profile:" << profileName );

    if (session->isScheduled() && iLowPower != 0 && iLowPower->value().toBool())
    {
        LOG_DEBUG( "Low power, scheduled sync aborted" );
        iSyncQueue.dequeue();
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
        cleanupSession(session);
        emit syncStatus(profileName, Sync::SYNC_ERROR, "", 0);
        tryNext = true;
    }
    else if (!isTransportAvailable(session))
    {
        LOG_DEBUG( "Required transport not available, aborting sync request" );
        iSyncQueue.dequeue();
        session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
        cleanupSession(session);
        emit syncStatus(profileName, Sync::SYNC_ERROR, "", 0);
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
            session->setFailureResult(SyncResults::SYNC_RESULT_FAILED);
            cleanupSession(session);
            emit syncStatus(profileName, Sync::SYNC_ERROR, "", 0);
        }
        tryNext = true;
    }

    return tryNext;
}

void Synchronizer::cleanupSession(SyncSession *aSession)
{
    FUNCTION_CALL_TRACE;

    if (aSession != 0)
    {
        QString profileName = aSession->profileName();
        if (!profileName.isEmpty())
        {
	    if (aSession->iCreateProfile)	
		    iProfileManager.saveRemoteTargetId(*aSession->profile(), aSession->results().getTargetId());
            iProfileManager.saveSyncResults(profileName, aSession->results());

            if (aSession->isScheduled())
            {
                reschedule(profileName);
            } // no else
        } // no else
        aSession->iCreateProfile = false;
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
    }
}

void Synchronizer::profileChanged(QString aProfileName)
{
    FUNCTION_CALL_TRACE;

    // @todo: remove
    // For testing purposes only
    /*
    if (aProfileName == "usb")
    {
        iTransportTracker->updateState(Sync::CONNECTIVITY_USB,
            !iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_USB));
    }
    else if (aProfileName == "bt")
    {
        iTransportTracker->updateState(Sync::CONNECTIVITY_BT,
            !iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_BT));
    }
    else if (aProfileName == "internet")
    {
        iTransportTracker->updateState(Sync::CONNECTIVITY_INTERNET,
            !iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_INTERNET));
    }
    else if (aProfileName == "addref")
    {
        iServerActivator->addRef("syncml");
    }
    else if (aProfileName == "removeref")
    {
        iServerActivator->removeRef("syncml");
    }
    else
    */
    {
        reschedule(aProfileName);
    }
}

void Synchronizer::profileDeleted(QString aProfileName)
{
    FUNCTION_CALL_TRACE;

    if (aProfileName.isEmpty())
	    return;
    
    SyncProfile *profile = iProfileManager.syncProfile (aProfileName);
    
    if (!profile)
	    return;
   
    bool client = true ; 
    Profile *subProfile = profile->clientProfile(); // client or server
    if ( !subProfile) {
        LOG_WARNING( "Could not find client sub-profile" );
	subProfile = profile->serverProfile ();
	client = false;
	if (!subProfile){
		LOG_WARNING( "Could not find server sub-profile" );
		return;
	}
    } 
    
    if (profile->syncType() == SyncProfile::SYNC_SCHEDULED){
               iSyncScheduler->removeProfile(aProfileName);
    }
    
    PluginRunner *pluginRunner ;
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
        return ;
    }

    if (!pluginRunner->cleanUp()){
    	LOG_CRITICAL ("Error in removing anchors");
    } else {
	LOG_DEBUG("Removing the profile");	    
	iProfileManager.remove (aProfileName, profile->type());
    }
    delete profile;
    delete pluginRunner;

    return;
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
    const QString &aMimeType )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Sync session progress" );
    LOG_DEBUG( "Profile:" << aProfileName );
    LOG_DEBUG( "Database:" << aDatabase );
    LOG_DEBUG( "Transfer type:" << aType );
    LOG_DEBUG( "Mime type:" << aMimeType );

    emit transferProgress( aProfileName, aDatabase, aType, aMimeType );

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
			if (aMimeType.compare(QString("text/x-vcard")) == 0)
				session->aStorageMap["hcontacts"] = true;
			else if (aMimeType.compare(QString("text/x-vcalendar")) == 0)
				session->aStorageMap["hcalendar"] = true;
			else if (aMimeType.compare(QString("text/plain")) == 0)
				session->aStorageMap["hnotes"] = true;
			else if (aMimeType.compare(QString("text/x-vbookmark")) == 0)
				session->aStorageMap["hbookmarks"] = true;
			else if (aMimeType.compare(QString("text/x-vmsg")) == 0)
				session->aStorageMap["hsms"] = true;
			else 
				LOG_DEBUG( "Unsupported mime type" << aMimeType );
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

void Synchronizer::startServers()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Starting server plug-ins" );

    if (iServerActivator != 0)
    {
        connect(iServerActivator, SIGNAL(serverEnabled(const QString &)),
            this, SLOT(startServer(const QString &)), Qt::QueuedConnection);

        connect(iServerActivator, SIGNAL(serverDisabled(const QString &)),
            this, SLOT(stopServer(const QString &)), Qt::QueuedConnection);

        QStringList enabledServers = iServerActivator->enabledServers();
        foreach (QString server, enabledServers)
        {
            startServer(server);
        }
    }
    else
    {
        LOG_CRITICAL("No server plug-in activator");
    }
}

void Synchronizer::stopServers()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Stopping all server plug-ins" );

    QStringList activeServers = iServers.keys();
    foreach (QString server, activeServers)
    {
        stopServer(server);
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

        SyncSession *session = new SyncSession(profile, this);
        if (session != 0)
        {
	    session->iCreateProfile = createNewProfile;
	    // disable all storages
	    session->aStorageMap["hcontacts"] = 0;
	    session->aStorageMap["hcalendar"] = 0;
	    session->aStorageMap["hnotes"] = 0;
	    session->aStorageMap["hbookmarks"] = 0;
	    session->aStorageMap["hsms"] = 0;
	    	  	    
            iActiveSessions.insert(profile->name(), session);

            // Connect signals from sync session.
            connect(session, SIGNAL(transferProgress(const QString &,
                Sync::TransferDatabase, Sync::TransferType, const QString &)),
                this, SLOT(onTransferProgress(const QString &,
                Sync::TransferDatabase, Sync::TransferType, const QString &)));
            connect(session, SIGNAL(storageAccquired(const QString &, const QString &)),
                this, SLOT(onStorageAccquired(const QString &, const QString &)));
            connect(session, SIGNAL(finished(const QString &, Sync::SyncStatus,
                const QString &, int)),
                this, SLOT(onSessionFinished(const QString &, Sync::SyncStatus,
                const QString &, int)));

            // Associate plug-in runner with the new session.
            session->setPluginRunner(pluginRunner, false);
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
    if (profile && profile->syncType() == SyncProfile::SYNC_SCHEDULED)
    {
        iSyncScheduler->addProfile(profile);
        delete profile;
        profile = NULL;
    }
    else
    {
        iSyncScheduler->removeProfile(aProfileName);
    }
}

bool Synchronizer::isTransportAvailable(const SyncSession *aSession)
{
    bool available = false;
    if (aSession != 0 && iTransportTracker != 0 && aSession->profile() != 0)
    {
        switch (aSession->profile()->destinationType())
        {
        case SyncProfile::DESTINATION_TYPE_DEVICE:
            available = iTransportTracker->isConnectivityAvailable(
                Sync::CONNECTIVITY_BT);
            break;

        case SyncProfile::DESTINATION_TYPE_ONLINE:
            available = iTransportTracker->isConnectivityAvailable(
                Sync::CONNECTIVITY_INTERNET);
            break;

        default:
            LOG_DEBUG("Destination type not defined, assuming transport available");
            available = true;
            break;
        }
    }

    return available;
}
