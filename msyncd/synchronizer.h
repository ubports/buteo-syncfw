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


#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include "SyncDBusInterface.h"
#include "SyncQueue.h"
#include "StorageBooker.h"
#include "SyncScheduler.h"

#include "SyncCommonDefs.h"
#include "ProfileManager.h"
#include "PluginManager.h"
#include "PluginCbInterface.h"
#include "ClientPlugin.h"

#include <QVector>
#include <QMutex>
#include <QCoreApplication>
#include <QMap>
#include <QString>


class ContextProperty;

/*! \mainpage Harmattan Sync Framework
 *
* The Synchronization framework subsystem provides a generic framework for
* application developers wanting to develop Synchronization plug-ins for any
* synchronization service. It provides a plug-in API for the client, server and the storage
* plug-ins in the framework. It also provides services like profile management, sync
* controller functionality for sequencing simultaneous sync requests, a sync daemon and
* other common sync related functionality. This subsystem also provides an
* implementation of the OMA DS SyncML specification. The SyncML engine is provided as a
* public API for application developers. It also provides an implementation of the MTP 1.0
* specification, but this library interface is not open.
*
*/

namespace Buteo {

class PluginManager;
class ServerPluginRunner;
class TransportTracker;
class ServerActivator;
class AccountsHelper;

/// \brief The main entry point to the synchronization framework.
///
/// This class manages other components and connects them to provide
/// the fully functioning synchronization framework.
class Synchronizer : public SyncDBusInterface, // Derived from QObject
                     public PluginCbInterface
{
    Q_OBJECT

public:

    /// \brief The contructor.
    Synchronizer(QCoreApplication *aApplication);

    /// \brief Destructor
    virtual ~Synchronizer();

    bool initialize();

    void close();


// From PluginCbInterface
// ---------------------------------------------------------------------------

    virtual bool requestStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller);

    virtual void releaseStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller);

    virtual StoragePlugin* createStorage(const QString &aPluginName);

    virtual void destroyStorage(StoragePlugin *aStorage);

    virtual bool isConnectivityAvailable( Sync::ConnectivityType aType );


// From SyncDBusInterface
// --------------------------------------------------------------------------

public slots:

    //! \see SyncDBusInterface::startSync
    virtual bool startSync(QString aProfileName);

    //! \see SyncDBusInterface::abortSync
    virtual void abortSync(QString aProfileName);

    //! \see SyncDBusInterface::profileChanged
    virtual void profileChanged(QString aProfileName);

    //! \see SyncDBusInterface::profileDeleted
    virtual void profileDeleted(QString aProfileName);
    
    //! \see SyncDBusInterface::requestStorages
    virtual bool requestStorages(QStringList aStorageNames);

    //! \see SyncDBusInterface::releaseStorages
    virtual void releaseStorages(QStringList aStorageNames);

    //! \see SyncDBusInterface::runningSyncs
    virtual QStringList runningSyncs();

// --------------------------------------------------------------------------

    bool startScheduledSync(QString aProfileName);

signals:

    void storageReleased();

private slots:

    /*! \brief Handler for storage released signal.
     *
     * Tries to start the next sync in queue, which may have been blocked
     * earlier by storage reservations.
     */
    void onStorageReleased();

    void onTransferProgress( const QString &aProfileName,
        Sync::TransferDatabase aDatabase, Sync::TransferType aType,
        const QString &aMimeType );

    void onSessionFinished( const QString &aProfileName,
        Sync::SyncStatus aStatus, const QString &aMessage, int aErrorCode );

    void onStorageAccquired(const QString &aProfileName, const QString &aMimeType);
    
    void onServerDone();

    void onNewSession(const QString &aDestination);

    /*! \brief Starts a server plug-in
     *
     * @param aProfileName Server profile name
     */
    void startServer(const QString &aProfileName);

    /*! \brief Stops a server plug-in
     *
     * @param aProfileName Server profile name
     */
    void stopServer(const QString &aProfileName);

private:

    bool startSync(const QString &aProfileName, bool aScheduled);

    /*! \brief Starts a sync with the given profile.
     *
     * \param aProfile Profile to use in sync. Ownership is transferred.
     *  The profile is automatically deleted when the sync finishes.
     */
    bool startSyncNow(SyncSession *aSession);

    /*! \brief Tries to starts next sync request from the sync queue.
     *
     * \return Is it possible to try starting more syncs by calling this
     *  function again. Will be true if the first sync request in the queue
     *  is not blocked by already reserved storages.
     */
    bool startNextSync();

    void cleanupSession(SyncSession *aSession);

    /*! \brief Start all server plug-ins
     *
     */
    void startServers();

    /*! \brief Stop all server plug-ins
     *
     */
    void stopServers();

    /*! \brief Adds a profile to sync scheduler
     *
     * @param aProfileName Name of the profile to schedule.
     */
    void reschedule(const QString &aProfileName);

    bool isTransportAvailable(const SyncSession *aSession);

    QMap<QString, SyncSession*> iActiveSessions;

    QMap<QString, ServerPluginRunner*> iServers;

    PluginManager iPluginManager;

    ProfileManager iProfileManager;

    SyncQueue iSyncQueue;

    StorageBooker iStorageBooker;

    SyncScheduler *iSyncScheduler;

    TransportTracker *iTransportTracker;

    ServerActivator *iServerActivator;

    AccountsHelper *iAccounts;

    ContextProperty *iLowPower;

    bool iClosing;

    friend class SynchronizerTest;
};

}

#endif // SYNCHRONIZER_H
