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
#include "SyncBackup.h"
#include "SyncOnChange.h"
#include "SyncOnChangeScheduler.h"

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
#include <QDBusInterface>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QDeviceInfo>
#include <QBatteryInfo>
#else
#include <QtSystemInfo/QSystemDeviceInfo>
#endif


namespace Buteo {

class PluginManager;
class ServerPluginRunner;
class NetworkManager;
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

    /// \brief registers the dbus service and creates handlers for various
    /// tasks of the synchronizer
    bool initialize();

    /// \brief stops the daemon and unregisters the dbus object
    void close();


// From PluginCbInterface
// ---------------------------------------------------------------------------
    /// \see PluginCbInterface::requestStorage
    virtual bool requestStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller);

    /// \see PluginCbInterface::releaseStorage
    virtual void releaseStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller);

    /// \see PluginCbInterface::createStorage
    virtual StoragePlugin* createStorage(const QString &aPluginName);

    /// \see PluginCbInterface::destroyStorage
    virtual void destroyStorage(StoragePlugin *aStorage);

    /// \see PluginCbInterface::isConnectivityAvailable
    virtual bool isConnectivityAvailable( Sync::ConnectivityType aType );

    /// \see PluginCbInterface::getSyncProfileByRemoteAddress
    virtual Profile* getSyncProfileByRemoteAddress(const QString& aAddress);

    /// \see PluginCbInterface::getValue
    virtual QString getValue(const QString& aAddress, const QString& aKey);


// From SyncDBusInterface
// --------------------------------------------------------------------------

public slots:

    //! \see SyncDBusInterface::startSync
    virtual bool startSync(QString aProfileName);

    //! \see SyncDBusInterface::abortSync
    virtual void abortSync(QString aProfileName);

    //! \see SyncDBusInterface::removeProfile
    virtual bool removeProfile(QString aProfileAsXml);

    //! \see SyncDBusInterface::updateProfile
    virtual bool updateProfile(QString aProfileAsXml);

    //! \see SyncDBusInterface::requestStorages
    virtual bool requestStorages(QStringList aStorageNames);

    //! \see SyncDBusInterface::releaseStorages
    virtual void releaseStorages(QStringList aStorageNames);

    //! \see SyncDBusInterface::runningSyncs
    virtual QStringList runningSyncs();

    //! \see SyncDBusInterface::setSyncSchedule
    virtual bool setSyncSchedule(QString aProfileId , QString aScheduleAsXml);

    //! \see SyncDBusInterface::saveSyncResults
    virtual bool saveSyncResults(QString aProfileId,QString aSyncResults);

    /*! \brief To get lastSyncResult.
     *  \param aProfileId
     *  \return QString of syncResult.
     */
    virtual QString getLastSyncResult(const QString &aProfileId);

    /*! \brief Gets all visible sync profiles.
     *
     * Returns all sync profiles that should be visible in sync ui. A profile
     * is visible if it has not been explicitly set as hidden.
     * \return The list of sync profiles.
     */
    virtual QStringList allVisibleSyncProfiles();

    /*! \brief Gets a sync profile.
     *
     * Loads and merges also all sub-profiles that are referenced from the
     * main profile. Loads the log of finished synchronization sessions with
     * this profile.
     * \param aProfileId Name of the profile to get.
     * \return The sync profile as Xml string.
     */
    virtual QString syncProfile(const QString &aProfileId);
    virtual QStringList syncProfilesByKey(const QString &aKey, const QString &aValue);
    virtual QStringList syncProfilesByType(const QString &aType);
// --------------------------------------------------------------------------

    //! Called  starts a schedule sync.
    bool startScheduledSync(QString aProfileName);

    //! Called  when backup starts
    void backupStarts();

    //! Called when backup is completed
    void backupFinished();

    //! Called when  starting to restore a backup.
    void restoreStarts();

    //! Called when backup is restored
    void restoreFinished();

    //! Called to get the current backup/restore state
    virtual bool getBackUpRestoreState();

    void start(unsigned int aAccountId);

    /*! \brief Stops sync for all profiles matching the given account ID.
     *
     * \param aAccountId The account ID.
     */
    void stop(unsigned int aAccountId);

    /*! \brief Returns the list of account IDs for which sync is ongoing
     *
     * \return The list of account IDs currectly syncing.
     */
    QList<unsigned int> syncingAccounts();

    /*! \brief Returns the status of the sync for the given account Id
     *
     * \param aAccountId The account ID.
     * \param aFailedReason This is an out parameter. In case the last sync has
     * failed, this will contain the code indicating the failure reason (TODO:
     * Define error codes). In case the last sync has not failed, this must be
     * ignored
     * \param aPrevSyncTime This is an out parameter. The previous sync time.
     * Invalid time is returned if there was no last sync.
     * \param aNextSyncTime This is an out parameter. The next sync time.
     * \return The status of sync: 0 = Sync is running,
     * 1 = Last sync succeeded, 2 = last sync failed
     */
    int status(unsigned int aAccountId, int &aFailedReason, qlonglong &aPrevSyncTime, qlonglong &aNextSyncTime);

signals:

        //! emitted by releaseStorages call
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
        const QString &aMimeType, int aCommittedItems );

    void onSessionFinished( const QString &aProfileName,
        Sync::SyncStatus aStatus, const QString &aMessage, int aErrorCode );

    void onStorageAccquired(const QString &aProfileName, const QString &aMimeType);

    void onSyncProgressDetail(const QString &aProfileName,int aProgressDetail);

    void onServerDone();

    void onNewSession(const QString &aDestination);

    void slotNetworkSessionOpened();

    void slotNetworkSessionError();

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

    void onNetworkStateChanged(bool aState);

    /*! \brief call this to request the sync daemon to enable soc
     * for a profile. The sync daemon decides as of now for which storages
     * soc should be enabled
     *
     * @param aProfileName profile name
     */
    void enableSOCSlot(const QString& aProfileName);

    /*! \brief Adds a profile to sync scheduler
     *
     * @param aProfileName Name of the profile to schedule.
     */
    void reschedule(const QString &aProfileName);

    /*! \brief Handles the sync status signal
     *
     * @param aProfileName Name of the profile
     * @param aStatus Status of the sync
     * @param aMessage Status message as a string
     * @param aMoreDetails In case of failure, contains detailed reason
     */
    void slotSyncStatus(QString aProfileName, int aStatus,
                        QString aMessage, int aMoreDetails);
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

    /*! \brief To clean up session
     *  \param aSession
     *  \param aStatus of sync
     *  \return None
     */
    void cleanupSession(SyncSession *aSession, Sync::SyncStatus aStatus);

    /*! \brief Start all server plug-ins
     *
     * @param resume, if true resume servers instead of starting them
     */
    void startServers( bool resume = false );

    /*! \brief Stop all server plug-ins
     *
     * @param suspend, if true suspend servers instead of stopping them
     */
    void stopServers( bool suspend = false );

    /*! \brief Helper function when backup/restore starts.
     *
     */
     void backupRestoreStarts ();

    /*! \brief Helper function when backup/restore is done.
     *
     */
     void backupRestoreFinished();

    /*! \brief Initializes sync scheduler
     *
     */
    void initializeScheduler();

    bool isBackupRestoreInProgress ();

    /*! \brief Requests for a cleanup from the plugin for the given profileId
     *
     * @param aProfileId Name/Id of the profile
     * @return True or False to indicate success or failure
     */
    bool cleanupProfile(const QString &profileId);

    QMap<QString, SyncSession*> iActiveSessions;

    QList<QString> iProfilesToRemove;

    QMap<QString, ServerPluginRunner*> iServers;

    QList<QString> iWaitingOnlineSyncs;

    NetworkManager *iNetworkManager;

    QMap<QString, int> iCountersStorage;

    PluginManager iPluginManager;

    ProfileManager iProfileManager;

    SyncQueue iSyncQueue;

    StorageBooker iStorageBooker;

    SyncScheduler *iSyncScheduler;

    SyncBackup *iSyncBackup;

    TransportTracker *iTransportTracker;

    ServerActivator *iServerActivator;

    AccountsHelper *iAccounts;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QBatteryInfo iDeviceInfo;
#else
    QtMobility::QSystemDeviceInfo iDeviceInfo;
#endif

    bool iClosing;

    SyncOnChange iSyncOnChange;

    SyncOnChangeScheduler iSyncOnChangeScheduler;

    /*! \brief Save the counter for given profile
     *
     * @param aProfile profile to save counter
     */
    void saveProfileCounter(const SyncProfile* aProfile);

    /*! \brief Restore the counter for given profile
     *
     * @param aProfile profile to restore counter
     */
    void restoreProfileCounter(SyncProfile* aProfile);

    bool iSOCEnabled;

    QString iUUID;

    QString iRemoteName;

#ifdef SYNCFW_UNIT_TESTS
    friend class SynchronizerTest;
#endif

    QDBusInterface *iSyncUIInterface;
};

}

#endif // SYNCHRONIZER_H
