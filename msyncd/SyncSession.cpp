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

#include "SyncSession.h"
#include "PluginRunner.h"
#include "StorageBooker.h"
#include "SyncProfile.h"
#include "NetworkManager.h"
#include "LogMacros.h"

using namespace Buteo;

SyncSession::SyncSession(SyncProfile *aProfile, QObject *aParent)
    :   QObject(aParent),
        iProfile(aProfile),
        iPluginRunner(0),
        iStatus(Sync::SYNC_ERROR),
        iErrorCode(0),
        iPluginRunnerOwned(false),
        iScheduled(false),
        iAborted(false),
        iStarted(false),
        iFinished(false),
        iCreateProfile(false),
        iStorageBooker(0),
        iNetworkManager(0)
{
    FUNCTION_CALL_TRACE;
}

SyncSession::~SyncSession()
{
    FUNCTION_CALL_TRACE;

    if (iPluginRunnerOwned) {
        PluginRunner *runner = iPluginRunner;
        iPluginRunner = 0;
        delete runner;
    }

    if (iNetworkManager) {
        // Disconnect all slots connected to the network manager
        disconnect(iNetworkManager, SIGNAL(connectionSuccess()),
                   this, SLOT(onNetworkSessionOpened()));
        disconnect(iNetworkManager, SIGNAL(connectionError()),
                   this, SLOT(onNetworkSessionError()));
        iNetworkManager->disconnectSession();
    }

    releaseStorages();

    delete iProfile;
    iProfile = 0;
}

void SyncSession::setPluginRunner(PluginRunner *aPluginRunner,
                                  bool aTransferOwnership)
{
    FUNCTION_CALL_TRACE;

    // Delete old owned plug-in runner, if any.
    if (iPluginRunnerOwned) {
        delete iPluginRunner;
        iPluginRunner = 0;
    }

    iPluginRunner = aPluginRunner;
    iPluginRunnerOwned = aTransferOwnership;
    if (iPluginRunner != 0) {
        // As we are setting a plugin runner, the pluginrunner should have
        // been started already
        iStarted = true;
        // Connect signals from plug-in runner.
        connect(iPluginRunner, SIGNAL(transferProgress(const QString &,
                                                       Sync::TransferDatabase, Sync::TransferType, const QString &, int)),
                this, SLOT(onTransferProgress(const QString &, Sync::TransferDatabase,
                                              Sync::TransferType, const QString &, int)));
        connect(iPluginRunner, SIGNAL(error(const QString &, const QString &, int)),
                this, SLOT(onError(const QString &, const QString &, int)));
        connect(iPluginRunner, SIGNAL(success(const QString &, const QString &)),
                this, SLOT(onSuccess(const QString &, const QString &)));
        connect(iPluginRunner, SIGNAL(storageAccquired(const QString &)),
                this, SLOT(onStorageAccquired(const QString &)));
        connect(iPluginRunner, SIGNAL(syncProgressDetail(const QString &, int)),
                this, SLOT(onSyncProgressDetail(const QString &, int)));
        connect(iPluginRunner, SIGNAL(done()), this, SLOT(onDone()));
        connect(iPluginRunner, SIGNAL(destroyed(QObject *)),
                this, SLOT(onDestroyed(QObject *)));

    }
}

PluginRunner *SyncSession::pluginRunner()
{
    FUNCTION_CALL_TRACE;

    return iPluginRunner;
}

bool SyncSession::start()
{
    FUNCTION_CALL_TRACE;

    bool rv = false;
    // If this is an online session, then we need to ensure that the network
    // session is opened before starting our plugin runner

    if ((iProfile->destinationType() == SyncProfile::DESTINATION_TYPE_ONLINE) && !iScheduled) {
        iNetworkManager = new NetworkManager(this);
        connect(iNetworkManager, SIGNAL(connectionSuccess()),
                SLOT(onNetworkSessionOpened()), Qt::QueuedConnection);
        connect(iNetworkManager, SIGNAL(connectionError()),
                SLOT(onNetworkSessionError()), Qt::QueuedConnection);
        // Return true here and wait for the session open status
        iNetworkManager->connectSession(iScheduled);
        rv = true;
    } else {
        rv = tryStart();
    }
    return rv;
}

bool SyncSession::tryStart()
{
    bool rv = false;

    if (iPluginRunner != 0) {
        iStarted = rv = iPluginRunner->start();
    }

    if (!rv) {
        updateResults(SyncResults(QDateTime::currentDateTime(),
                                  SyncResults::SYNC_RESULT_FAILED,
                                  Buteo::SyncResults::INTERNAL_ERROR));

        if (iPluginRunner != 0) {
            disconnect(iPluginRunner, 0, this, 0);
        }
    }
    return rv;
}

bool SyncSession::isFinished()
{
    return iFinished;
}

bool SyncSession::isAborted()
{
    return iAborted;
}

void SyncSession::abort(Sync::SyncStatus aStatus)
{
    FUNCTION_CALL_TRACE;

    if (!iStarted) {
        LOG_DEBUG("Client plugin runner not started, ignore abort");
        updateResults(SyncResults(QDateTime::currentDateTime(),
                                  SyncResults::SYNC_RESULT_FAILED,
                                  Buteo::SyncResults::ABORTED));
        emit finished(profileName(), Sync::SYNC_ERROR, QString(), SyncResults::ABORTED);
        return;
    } else {
        iAborted = true;

        if (iPluginRunner != 0) {
            iPluginRunner->abort(aStatus);
        }
    }
}

QMap<QString, bool> SyncSession::getStorageMap()
{
    FUNCTION_CALL_TRACE
    return iStorageMap;
}

void SyncSession::setStorageMap(QMap<QString, bool> &aStorageMap)
{
    FUNCTION_CALL_TRACE
    iStorageMap = aStorageMap;
}

bool SyncSession::isProfileCreated()
{
    FUNCTION_CALL_TRACE
    return iCreateProfile;
}

void SyncSession::setProfileCreated(bool aProfileCreated)
{
    FUNCTION_CALL_TRACE
    iCreateProfile = aProfileCreated;
}

void SyncSession::stop()
{
    FUNCTION_CALL_TRACE;

    if (!iStarted) {
        LOG_DEBUG("Plugin runner not yet started, ignoring stop.");
    } else if (iPluginRunner != 0) {
        iPluginRunner->stop();
    }
}

SyncProfile *SyncSession::profile() const
{
    FUNCTION_CALL_TRACE;

    return iProfile;
}

QString SyncSession::profileName() const
{
    FUNCTION_CALL_TRACE;

    QString name;
    if (iProfile != 0) {
        name = iProfile->name();
    } // no else

    return name;
}

SyncResults SyncSession::results() const
{
    FUNCTION_CALL_TRACE;

    return iResults;
}


void SyncSession::setScheduled(bool aScheduled)
{
    FUNCTION_CALL_TRACE;

    iScheduled = aScheduled;
    iResults.setScheduled(iScheduled);
}

bool SyncSession::isScheduled() const
{
    FUNCTION_CALL_TRACE;

    return iScheduled;
}

void SyncSession::onSuccess(const QString &aProfileName, const QString &aMessage)
{
    FUNCTION_CALL_TRACE;
    iErrorCode = 0;

    Q_UNUSED(aProfileName);

    iFinished = true;
    if (!iAborted) {
        iStatus = Sync::SYNC_DONE;
    } else {
        iStatus = Sync::SYNC_ABORTED;
    }
    iMessage = aMessage;

    if (iPluginRunner != 0) {
        updateResults(iPluginRunner->syncResults());
    }
    emit finished(profileName(), iStatus, iMessage, iErrorCode);

}

void SyncSession::onError(const QString &aProfileName, const QString &aMessage,
                          int aErrorCode)
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aProfileName);

    iFinished = true;
    iStatus = mapToSyncStatusError(aErrorCode);
    iMessage = aMessage;
    iErrorCode = aErrorCode;

    if (iPluginRunner != 0) {
        updateResults(iPluginRunner->syncResults());
    }
    emit finished(profileName(), iStatus, iMessage, iErrorCode);
}

Sync::SyncStatus SyncSession::mapToSyncStatusError(int aErrorCode)
{
    Sync::SyncStatus status;
    SyncResults::MinorCode code = static_cast<SyncResults::MinorCode>(aErrorCode);

    switch (code) {
    case SyncResults::UNSUPPORTED_SYNC_TYPE: {
        status = Sync::SYNC_NOTPOSSIBLE;
        break;
    }
    default: {
        status = Sync::SYNC_ERROR;
        break;
    }
    }
    return status;
}

void SyncSession::onTransferProgress(const QString &aProfileName,
                                     Sync::TransferDatabase aDatabase, Sync::TransferType aType,
                                     const QString &aMimeType, int aCommittedItems)
{
    FUNCTION_CALL_TRACE;

    emit transferProgress(aProfileName, aDatabase, aType, aMimeType, aCommittedItems);
}

void SyncSession::onStorageAccquired (const QString &aMimeType)
{
    FUNCTION_CALL_TRACE;
    emit storageAccquired (profileName(), aMimeType);
}

void SyncSession::onSyncProgressDetail(const QString &aProfileName, int aProgressDetail)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED(aProfileName);
    emit syncProgressDetail (profileName(), aProgressDetail);
}

void SyncSession::onDone()
{
    FUNCTION_CALL_TRACE;

    QString pluginName;
    if (iPluginRunner != 0) {
        disconnect(iPluginRunner, 0, this, 0);
        pluginName = iPluginRunner->pluginName();
    }

    if (!iFinished) {
        LOG_WARNING("Plug-in terminated unexpectedly:" << pluginName);
        emit finished(profileName(), Sync::SYNC_ERROR, iMessage, 0);
    }
}

void SyncSession::onDestroyed(QObject *aPluginRunner)
{
    if (iPluginRunner == aPluginRunner) {
        LOG_WARNING("Plug-in runner destroyed before sync session");
        iPluginRunner = 0;
    }
}

void SyncSession::updateResults(const SyncResults &aResults)
{
    FUNCTION_CALL_TRACE;
    iResults = aResults;
    iResults.setScheduled(iScheduled);
    iResults.setTargetId(aResults.getTargetId());
}

void SyncSession::setFailureResult(int aMajorCode, int aMinorCode)
{
    FUNCTION_CALL_TRACE;

    iResults.setMajorCode(aMajorCode);
    iResults.setMinorCode(aMinorCode);
}

bool SyncSession::reserveStorages(StorageBooker *aStorageBooker)
{
    FUNCTION_CALL_TRACE;

    bool success = false;
    if (aStorageBooker != 0 && iProfile != 0 &&
            aStorageBooker->reserveStorages(iProfile->storageBackendNames(),
                                            iProfile->name())) {
        success = true;
        iStorageBooker = aStorageBooker;
    }

    return success;
}

void SyncSession::releaseStorages()
{
    // Release storages that were reserved earlier.
    if (iStorageBooker != 0 && iProfile != 0) {
        iStorageBooker->releaseStorages(iProfile->storageBackendNames());
    }

    // Set storage booker to NULL. This indicates that we don't hold any
    // storage reservations.
    iStorageBooker = 0;
}

void SyncSession::onNetworkSessionOpened()
{
    // Start the plugin runner now
    FUNCTION_CALL_TRACE;

    if (iNetworkManager) {
        // Disconnect all slots connected to the network manager
        disconnect(iNetworkManager, SIGNAL(connectionSuccess()),
                   this, SLOT(onNetworkSessionOpened()));
        disconnect(iNetworkManager, SIGNAL(connectionError()),
                   this, SLOT(onNetworkSessionError()));
    }

    if (false == tryStart()) {
        LOG_WARNING("attempt to start sync session due to network session opened failed!");
        updateResults(SyncResults(QDateTime::currentDateTime(),
                                  SyncResults::SYNC_RESULT_FAILED,
                                  Buteo::SyncResults::INTERNAL_ERROR));
        emit finished(profileName(), Sync::SYNC_ERROR, QString(), SyncResults::INTERNAL_ERROR);
    } else {
        LOG_DEBUG("attempt to start sync session due to network session opened succeeded.");
    }
}

void SyncSession::onNetworkSessionError()
{
    FUNCTION_CALL_TRACE;

    if (iNetworkManager) {
        // Disconnect all slots connected to the network manager
        disconnect(iNetworkManager, SIGNAL(connectionSuccess()),
                   this, SLOT(onNetworkSessionOpened()));
        disconnect(iNetworkManager, SIGNAL(connectionError()),
                   this, SLOT(onNetworkSessionError()));
        iNetworkManager->disconnectSession();
    }

    updateResults(SyncResults(QDateTime::currentDateTime(),
                              SyncResults::SYNC_RESULT_FAILED,
                              Buteo::SyncResults::CONNECTION_ERROR));
    // Update the session with connection error
    emit finished(profileName(), Sync::SYNC_ERROR, QString(), SyncResults::CONNECTION_ERROR);
}

