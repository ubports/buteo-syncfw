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

#ifndef SYNCSESSION_H
#define SYNCSESSION_H

#include "SyncCommonDefs.h"
#include "SyncResults.h"
#include <QObject>
#include <QMap>

namespace Buteo {

class SyncProfile;
class PluginRunner;
class StorageBooker;
class NetworkManager;

/*! \brief Class representing a single sync session
 *
 * The session can be initiated by a client or server plug-in
 */
class SyncSession : public QObject
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * @param aProfile SyncProfile associated with the session. With server
     *  plug-in initiated sessions this can be null, if there is no profile with
     *  a matching destination address.
     * @param aParent Parent object
     */
    explicit SyncSession(SyncProfile *aProfile, QObject *aParent = 0);

    //! \brief Destructor
    virtual ~SyncSession();

    /*! \brief Associates a plug-in runner with this session
     *
     * @param aPluginRunner The plug-in runner to use for this session
     * @param aTransferOwnership Does this session become the owner of the plug-in
     *  runner instance. If so, the plug-in runner is deleted when the session is
     *  deleted.
     */
    void setPluginRunner(PluginRunner *aPluginRunner, bool aTransferOwnership);

    /*! \brief Gets the plug-in runner associated with this session
     *
     * @return Plug-in runner
     */
    PluginRunner *pluginRunner();

    /*! \brief Returns if the sync session is finished or in process
     *
     * @return Finished indicator
     */
    bool isFinished();

    /*! \brief Returns if the sync session was aborted
     *
     * @return Aborted indicator
     */
    bool isAborted();

    /*! \brief Starts the session using the associated plug-in runner
     *
     * @return Success indicator
     */
    bool start();

    /*! \brief Aborts the session. Returns when the abort request is sent.
     *  \param - Status.
     */
    void abort(Sync::SyncStatus aStatus = Sync::SYNC_ABORTED);

    /*! \brief Stops the session. Returns when the session is stopped.
     */
    void stop();

    /*! \brief Gets the sync profile used by this session
     *
     * @return Sync profile
     */
    SyncProfile *profile() const;

    /*! \brief Gets the name of the profile used by this session
     *
     * @return Profile name
     */
    QString profileName() const;

    /*! \brief Gets the results of the finished session.
     *
     * This function should be called only after the finished signal is received
     * from the session.
     * @return Sync results
     */
    SyncResults results() const;

    /*! \brief Sets if the session was started by the scheduler
     *
     * @param  aScheduled True if scheduled, false otherwise
     */
    void setScheduled(bool aScheduled);

    /*! \brief Checks if the session was started by the scheduler
     *
     * @return True if scheduled, false otherwise
     */
    bool isScheduled() const;

    /*! \brief Sets the results for this session
     *
     * This function can be used in error situations to set the results to this
     * session, even if the session has failed to run.
     * @param aResults The results to set
     */
    void updateResults(const SyncResults &aResults);

    /*! \brief Sets the results for this session using the provided error code
     *
     * This function can be used in error situations to set the results to this
     * session, even if the session has failed to run. Time stamp for the results
     * is set to current time.
     * \param aMajorCode Error code
     * \param aMinorCode failed reason
     */
    void setFailureResult(int aMajorCode, int aMinorCode);

    /*! \brief Tries to reserve storages needed by the session
     *
     * Successfully reserved storages are automatically released when the session
     * is deleted.
     * @param aStorageBooker Storake booker to use for reserving the storages.
     *  If reserving is successfull, this booker is saved internally and used
     *  later to release the storages when the session is deleted.
     * @return Success indicator. True if all storages were successfully reserved.
     *  When false, no storages were reserved, meaning one or more of the needed
     *  storages were already in use.
     */
    bool reserveStorages(StorageBooker *aStorageBooker);

    //! \brief Releases storages that were reserved earlier with reserveStorages
    void releaseStorages();

    //! \brief returns the StorageMap used for this session
    QMap<QString,bool> getStorageMap();

    /*! \brief sets the storage map for this session
     *
     * @param aStorageMap - storage map to set
     */
    void setStorageMap(QMap<QString,bool> &aStorageMap);

    //! \brief returns the returns the status of the profile creation for this session
    bool isProfileCreated();

    //! \brief sets Profile Created flag to true
    void setProfileCreated(bool aProfileCreated);

    //! \brief Maps sync failure error code from stack to SyncStatus
        Sync::SyncStatus mapToSyncStatusError(int aErrorCode);

signals:

    //! @see SyncPluginBase::transferProgress
    void transferProgress(const QString &aProfileName,
            Sync::TransferDatabase aDatabase, Sync::TransferType aType,
            const QString &aMimeType, int aCommittedItems);

    /*! \brief Signal sent when a storage is accquired
     *
     * @param aProfileName Name of the profile used by the session
     * @param aMimeType Mimetype of the storage accquired.
     */
    void storageAccquired (const QString &aProfileName , const QString &aMimeType) ;

    /*! \brief Signal sent when the session has finished
     *
     * @param aProfileName Name of the profile used by the session
     * @param aStatus Status of the finished session
     * @param aMessage Possible textual message
     * @param aErrorCode Error code, if the status is error
     */
    void finished(const QString &aProfileName, Sync::SyncStatus aStatus,
            const QString &aMessage, int aErrorCode);

    /*! \brief Signal sent when the sync is in progress to indicate the detail of the progress
     *
     * @param aProfileName Name of the profile used by the session
     * @param aProgressDetail Detail of the progress.
     */
    void syncProgressDetail(const QString &aProfileName,int aProgressDetail);
private:

    bool tryStart();

private slots:

    // Slots for catching plug-in runner signals.

    void onSuccess(const QString &aProfileName, const QString &aMessage);

    void onError(const QString &aProfileName, const QString &aMessage, int aErrorCode);

    void onTransferProgress(const QString &aProfileName,
            Sync::TransferDatabase aDatabase, Sync::TransferType aType,
            const QString &aMimeType, int aCommittedItems);

    void onStorageAccquired (const QString &aMimeType);

    void onSyncProgressDetail(const QString &aProfileName,int aProgressDetail);

    void onDone();

    void onDestroyed(QObject *aPluginRunner);

    void onNetworkSessionOpened();

    void onNetworkSessionError();

    private:

    SyncProfile *iProfile;

    PluginRunner *iPluginRunner;

    SyncResults iResults;

    Sync::SyncStatus iStatus;

    int iErrorCode;

    bool iPluginRunnerOwned;

    bool iScheduled;

    bool iAborted;

    bool iStarted;

    bool iFinished;

    bool iCreateProfile;

    QString iMessage;

    QString iRemoteId ;

    StorageBooker *iStorageBooker;

    QMap<QString , bool> iStorageMap;

    NetworkManager *iNetworkManager;

    #ifdef SYNCFW_UNIT_TESTS
    friend class SyncSessionTest;
    #endif

};

}

#endif // SYNCSESSION_H
