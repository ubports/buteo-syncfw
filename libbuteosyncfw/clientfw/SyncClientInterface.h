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

#ifndef SYNCCLIENTINTERFACE_H
#define SYNCCLIENTINTERFACE_H

#include <QObject>
#include <QString>
#include <libsyncprofile/Profile.h>
#include <libsyncprofile/SyncProfile.h>
#include <libsyncprofile/SyncResults.h>
#include <libsyncprofile/SyncSchedule.h>


namespace Buteo {

class SyncClientInterfacePrivate;

/*! \brief
 * SyncInterface Class - Main Entry Point for SyncFW Clients
 *
 * This Class Provides Interface API towards SyncFW.
 * This Class has Methods to start and abort a sync , set schedule for a profile
 * and use the fw functionality to automatically schedule it, get a list of
 * running syncs( to display in UI ) and get status of the synchronization.
 * It also has Signals that the clients can connect to know the last sync results ,
 * to keep track of profile changes by SyncFw.
 */
class SyncClientInterface: public QObject
{
	Q_OBJECT

public:
	/*!
	 * \brief Constructor
	 */
	SyncClientInterface();

	/*!
	 * \brief Destructor
	 */
	~SyncClientInterface();

    /*!
     * \brief Requests to starts synchronizing using a profile Id
     *
     * A status change signal (QUEUED, STARTED or ERROR) will be sent by the
     * daemon when the request is processed. If there is a sync already in
     * progress using the same resources that are needed by the given profile,
     * adds the sync request to a sync queue. Otherwise a sync session is
     * started immediately.
     *
     * \param aProfileId Id of the profile to use in sync.
     * \return True if a profile with the Id was found. Otherwise
     *  false and no status change signals will follow from this request.
     */
    bool startSync(const QString &aProfileId) const;

    /*!
     * \brief Stops synchronizing the profile with the given Id.
     *
     * If the sync request is still in queue and not yet started, the queue
     * entry is removed.
     *
     * \param aProfileId Id of the profile to stop syncing.
     */
    void abortSync(const QString &aProfileId) const;

    /*!
     * \brief Gets the list of profile names of currently running syncs.
     *
     * \return Profile name list.
     */
    QStringList getRunningSyncList();


    /*!
     * \brief Sets Sync Schedule to the profile
     *
     * This function does the following
     * 1. sets the sync type of the profile to schedule
     * 2. Adds the schedule to the profile
     * 3. saves the profile.
     * 4. schedules the synchronization , so that sync automatically starts next time
     *
     * \return status of the operation
     */
    bool setSyncSchedule(QString &aProfileId,SyncSchedule &aSchedule);

    /*!
     * \brief Save SyncResults to log.xml file.
     * \param aProfileId to save result in corresponding file.
     * \param aSyncResults to save in the \code <profileId>.log.xml \endcode
     * \return status of the saveSyncResults
     */
    bool saveSyncResults(const QString &aProfileId,const Buteo::SyncResults &aSyncResults);

    /*!
     * \brief This function should be called when sync profile has to be deleted
     *
     * \param aProfileId Id of the profile to be deleted.
     * \return status of the remove operation
     */
    bool removeProfile(QString &aProfileId);

    /*!
     * \brief This function should be called when sync profile information has
     *  been changed by the client
     *
     * \param aSyncProfile Modified Profile Object.If same profile already exists it
     * will be overwritten with the changes from this object.
     * \return status of the update operation
     *
     */
    bool updateProfile(Buteo::SyncProfile &aSyncProfile);
	
	/*!
    * \brief This function returns true if backup/restore in progress else
    * false.
    */
    bool  getBackUpRestoreState();

    /*!
     * \brief Use this function to understand if the creation of dbus connection to msyncd
     * 		  succeeded or not.
     * \return  - status of the dbus object created for msyncd
     */
    bool isValid();

    /*! \brief To get lastSyncResult.
     *  \param aProfileId
     *  \return SyncResults of syncLastResult.
     */
    Buteo::SyncResults getLastSyncResult(const QString &aProfileId);

    /*! \brief Gets all visible sync profiles.
     *
     * Returns all sync profiles that should be visible in sync ui. A profile
     * is visible if it has not been explicitly set as hidden.
     * \return The list of sync profiles.
     */
    QList<QString /*profileAsXml*/> allVisibleSyncProfiles();

    /*! \brief Gets a sync profile.
     *
     * Loads and merges also all sub-profiles that are referenced from the
     * main profile. Loads the log of finished synchronization sessions with
     * this profile.
     * \param aProfileId Name of the profile to get.
     * \return The sync profile as Xml string.
     */
    QString syncProfile(const QString &aProfileId);
    
    /*! \brief Gets a sync profiles which matches the key-value.
     *
     * Loads and merges also all sub-profiles that are referenced from the
     * main profile. Loads the log of finished synchronization sessions with
     * this profile.
     * \param aKey Key to match for profile.
     * \param aValue Value to match for profile.
     * \return The sync profiles as Xml string list.
     */
    QStringList syncProfilesByKey(const QString &aKey, const QString &aValue);
    
    /*! \brief Gets a profiles  matching the profile type.
     *
     * \param aType Type of the profile service/storage/sync.
     * \return The sync profile ids as string list.
     */
    QStringList syncProfilesByType(const QString &aType);
signals:

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

	/*! \brief Notifies about a change in profile.
	 *
	 * This signal is sent when the profile data is modified or when a profile
	 * is added or deleted in msyncd.
	 * \param aProfileId Id of the changed profile.
	 * \param aChangeType
	 *      0 (ADDITION): Profile was added.
	 *      1 (MODIFICATION): Profile was modified.
	 *      2 (DELETION): Profile was deleted.
     * \param aChangedProfile changed sync profie as XMl string.
	 *
	 */
    void profileChanged(QString aProfileId,int aChangeType, QString aChangedProfile);

	/*! \brief Notifies about the results of a recent sync for a profile
	 *
	 * This signal is sent after the sync has completed for a profile.
	 * \param aProfileId Id of the changed profile.
	 * \param aResults -  Results of the sync
	 *
	 */
    void resultsAvailable(QString aProfileId  , Buteo::SyncResults aResults);

    /*!
     * \brief Notifies about a change in synchronization status.
     *
     * \param aProfileId Id of the profile used in the sync session whose
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
         * \param aStatusDetails
     *  When aStatus is ERROR, this parameter contains a specific error code.
     *  When aStatus is PROGRESS, this parameter contains more details about the progress
     *  \see SyncCommonDefs::SyncProgressDetails
     */
    void syncStatus(QString aProfileId, int aStatus,
                    QString aMessage, int aStatusDetails);

    /*! \brief Notifies about progress in transferring items
     *
     * \param aProfileId Id of the profile where progress has occurred
     * \param aTransferDatabase Database to which transfer was made. One of the following:
     *      0 (LOCAL_DATABASE): Transfer was made from remote database to local database
     *      1 (REMOTE_DATABASE): Transfer was made from local database to remote database
     * \param aTransferType Type of transfer that was made. One of the following:
     *      0 (ADDITION): Addition was made to database
     *      1 (MODIFICATION): Modification was made to database
     *      2 (DELETION): Deletion was made to database
     *      3 (ERROR): Addition/Modification/Deletion was attempted, but it failed
     * \param aMimeType Mime type of the processed item
     * \param aCommittedItems No. of items committed for this operation
     */
    void transferProgress(QString aProfileId, int aTransferDatabase,
                          int aTransferType , QString aMimeType, int aCommittedItems );

private:

    SyncClientInterfacePrivate *d_ptr;
};


};


#endif
