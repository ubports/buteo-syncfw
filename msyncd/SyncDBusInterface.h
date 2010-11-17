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
#ifndef SYNCDBUSINTERFACE_H
#define SYNCDBUSINTERFACE_H

#include <QtDBus>
#include <QObject>
#include <QString>

namespace Buteo {

/*!
 * \brief Defines a D-Bus interface for the sync daemon.
 *
 * A XML file describing the interface can be generated from this class using
 * qdbuscpp2xml tool. This XML file can then be used to generate interface
 * adaptor and proxy classes using qdbusxml2cpp tool.
 */
class SyncDBusInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meego.msyncd")

public:

signals:

    /*!
     * \brief Notifies about a change in synchronization status.
     *
     * \param aProfileName Name of the profile used in the sync session whose
     *  status has changed.
     * \param aStatus The new status. One of the following:
     *      0 (QUEUED): Sync request has been queued or was already in the
     *          queue when sync start was requested.
     *      1 (STARTED): Sync session has been started.
     *      2 (PROGRESS): Sync session is progressing.
     *      3 (ERROR): Sync session has encountered an error and has been stopped,
     *          or the session could not be started at all.
     *      4 (DONE): Sync session was successfully completed.
     *      5 (ABORTED): Sync session was aborted.
     *  Statuses 3-5 are final, no more status changes will be sent from the
     *  same sync session.
     * \param aMessage A message describing the status change in detail. This
     *  can for example be shown to the user or written to a log
     * \param aMoreDetails
     *  When aStatus is ERROR, this parameter contains a specific error code.
     *  When aStatus is PROGRESS, this parameter contains more details about the progress
     */
    void syncStatus(QString aProfileName, int aStatus,
                    QString aMessage, int aMoreDetails);

    /*! \brief Notifies about progress in transferring items
     *
     * \param aProfileName Name of the profile where progress has occurred
     * \param aTransferDatabase Database to which transfer was made. One of the following:
     *      0 (LOCAL_DATABASE): Transfer was made from remote database to local database
     *      1 (REMOTE_DATABASE): Transfer was made from local database to remote database
     * \param aTransferType Type of transfer that was made. One of the following:
     *      0 (ADDITION): Addition was made to database
     *      1 (MODIFICATION): Modification was made to database
     *      2 (DELETION): Deletion was made to database
     *      3 (ERROR): Addition/Modification/Deletion was attempted, but it failed
     * \param aMimeType Mime type of the processed item
     * \param aCommittedItems No. of Items committed for this operation
     */

    void transferProgress(QString aProfileName, int aTransferDatabase,
                          int aTransferType , QString aMimeType, int aCommittedItems);

    /*! \brief Notifies about a change in profile.
     *
     * This signal is sent when the profile data is modified or when a profile
     * is added or deleted in msyncd.
     * \param aProfileName Name of the changed profile.
     * \param aChangeType
     *      0 (ADDITION): Profile was added.
     *      1 (MODIFICATION): Profile was modified.
     *      2 (DELETION): Profile was deleted.
     * \param aProfileAsXml Updated Profile Object is sent as xml
     *
     */
    void signalProfileChanged(QString aProfileName, int aChangeType , QString aProfileAsXml);
    
   
    /*! \brief Notifies about Backup start.
     *
     * This signal is sent when the backup framework is backing the sync related
     * data
     */
    void backupInProgress ();
    
    /*! \brief Notifies about Backup done.
     *
     * This signal is sent when the backup framework has completed backing the sync related
     * data.
     */
    void backupDone();
    
    /*! \brief Notifies about Restore start.
     *
     * This signal is sent when the backup framework is restoring the sync related
     * data
     */
    void restoreInProgress();
    
    /*! \brief Notifies about Restore Done.
     *
     * This signal is sent when the backup framework has restored the sync related
     * data
     */
    void restoreDone();

    /*! \brief Notifies about the availability of Results for a recent sync
     *
     * This signal is sent when the results are available for the last sync
     * only recent results ( SyncResults object) are sent as xml.
     * \param aProfileName Name of the profile for which results are available
     * \param aResultsAsXml results as an xml object
     */
    void resultsAvailable(QString aProfileName , QString aResultsAsXml);
 
public slots:

    /*!
     * \brief Requests to starts synchronizing using a profile with the given
     *        name.
     *
     * A status change signal (QUEUED, STARTED or ERROR) will be sent by the
     * daemon when the request is processed. If there is a sync already in
     * progress using the same resources that are needed by the given profile,
     * adds the sync request to a sync queue. Otherwise a sync session is
     * started immediately.
     *
     * \param aProfileId Id of the profile to use in sync.
     * \return True if a profile with the given id was found. Otherwise
     *  false and no status change signals will follow from this request.
     */
    virtual bool startSync(QString aProfileId) = 0;

    /*!
     * \brief Stops synchronizing the profile with the given name.
     *
     * If the sync request is still in queue and not yet started, the queue
     * entry is removed.
     *
     * \param aProfileId Name of the profile to stop syncing.
     */
    virtual Q_NOREPLY void abortSync(QString aProfileId) = 0;

    /*!
     * \brief This function should be called when sync profile has to be deleted
     *
     * \param aProfileId Id of the profile to be deleted.
     * \return status of the remove operation
     */
    virtual bool removeProfile(QString aProfileId) = 0;

    /*!
     * \brief This function should be called when sync profile information has
     *  been changed by someone else than the sync daemon.
     * \note If profile does not exist prior to calling this function, a new profile file is created
     *
     * \param aProfileAsXml - Modified Profile Object as XML.
     * \return status of the update operation
     */
    virtual bool updateProfile(QString aProfileAsXml) = 0;

    /*!
     * \brief Requests sync daemon to reserve storages for the caller.
     *
     * This function must be called if an external sync entity (like Active
     * Sync engine) wants to use the same storages that the sync daemon uses,
     * because concurrent access might lead to data corruption. If none of the
     * requested storages is currently used by the sync daemon, they are all
     * marked as reserved and can not be used by the daemon until the storages
     * are freed by calling releaseStorages. If one or more of the requested
     * storages is already in use, none of them is reserved.
     *
     * \param aStorageNames Names of the storages to reserve.
     * \return Success indicator. True if all requested storages were
     *  successfully reserved. False if request failed and no storages were
     *  reserved.
     */
    virtual bool requestStorages(QStringList aStorageNames) = 0;

    /*!
     * \brief Releases the given storages so that sync daemon can again use
     *  them freely.
     *
     * This function must be called after a successful requestStorages call,
     * when the reserved storages are not used by the caller any more.
     */
    virtual Q_NOREPLY void releaseStorages(QStringList aStorageNames) = 0;

    /*!
     * \brief Gets the list of profile names of currently running syncs.
     *
     * \return Profile name list.
     */
    virtual QStringList runningSyncs() = 0;
    
    
    /*!
     * \brief This function returns true if backup/restore in progress else
     * false.
     */
    virtual bool  getBackUpRestoreState() = 0;
	
	
	/*!
     * \brief sets the schedule for a profile
     *
     * This Function helps in setting a schedule to profile
     * this Function is to be used by the SyncInterface Client Library to
     * expose a user friendly API  by abstracting the dbus mechanisms
     * involved with synchronizer
     *
     * \param aProfileId - Id of the profile for which schedule has to be set
     * \param aScheduleAsXml - Over the dbus the schedule object is transmitted as xml
     *
     * \return bool - status of the operation
     */
    virtual bool setSyncSchedule(QString aProfileId , QString aScheduleAsXml) = 0;

    /*!
     * \brief Save SyncResults to log.xml file.
     * \param aProfileId to save result in corresponding file.
     * \param aSyncResults to save in the \code <profileId>.log.xml. \endcode
     * \return status of the saveSyncResults
     */
    virtual bool saveSyncResults(QString aProfileId,QString aSyncResults) = 0;

    /*! \brief To get lastSyncResult.
     *  \param aProfileId
     *  \return QString of syncResult.
     */
    virtual QString getLastSyncResult(const QString &aProfileId) = 0;

    /*! \brief Gets all visible sync profiles.
     *
     * Returns all sync profiles that should be visible in sync ui. A profile
     * is visible if it has not been explicitly set as hidden.
     * \return The list of sync profiles.
     */
    virtual QStringList allVisibleSyncProfiles() = 0;

    /*! \brief Gets a sync profile.
     *
     * Loads and merges also all sub-profiles that are referenced from the
     * main profile. Loads the log of finished synchronization sessions with
     * this profile.
     * \param aProfileId Name of the profile to get.
     * \return The sync profile as Xml string.
     */
    virtual QString syncProfile(const QString &aProfileId) = 0;
    
    /*! \brief Gets a sync profiles matching the key-value.
     *
     * Loads and merges also all sub-profiles that are referenced from the
     * main profile. Loads the log of finished synchronization sessions with
     * this profile.
     * \param aKey Key to match for profile.
     * \param aValue Value to match for profile.
     * \return The sync profiles as Xml string list.
     */
    virtual QStringList syncProfilesByKey(const QString &aKey, const QString &aValue) = 0;
};

}

#endif // SYNCDBUSINTERFACE_H
