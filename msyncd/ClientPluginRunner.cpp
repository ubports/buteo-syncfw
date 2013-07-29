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

#include "ClientPluginRunner.h"
#include "ClientThread.h"
#include "ClientPlugin.h"
#include "LogMacros.h"
#include "PluginManager.h"

using namespace Buteo;

// Maximum time in milliseconds to wait for a thread to stop
static const unsigned long long MAX_THREAD_STOP_WAIT_TIME = 5000;

ClientPluginRunner::ClientPluginRunner(const QString &aPluginName,
    SyncProfile *aProfile, PluginManager *aPluginMgr,
    PluginCbInterface *aPluginCbIf, QObject *aParent)
:   PluginRunner(PLUGIN_CLIENT, aPluginName, aPluginMgr, aPluginCbIf, aParent),
    iProfile(aProfile),
    iPlugin(0),
    iThread(0)
{
    FUNCTION_CALL_TRACE;
}

ClientPluginRunner::~ClientPluginRunner()
{
    FUNCTION_CALL_TRACE;

    disconnect();

    if (iPlugin != 0 && iPluginMgr != 0)
    {
        iPluginMgr->destroyClient(iPlugin);
        iPlugin = 0;
    }

    if (iThread != 0)
    {
        delete iThread;
        iThread = 0;
    }
}

bool ClientPluginRunner::init()
{
    FUNCTION_CALL_TRACE;

    if (iInitialized)
        return true;

    if (iPluginMgr == 0 || iPluginCbIf == 0 || iProfile == 0)
    {
        LOG_WARNING("Invalid members, failed to initialize");
        return false;
    }

    iPlugin = iPluginMgr->createClient(iPluginName, *iProfile, iPluginCbIf);
    if (iPlugin == 0)
    {
        LOG_WARNING("Failed to create client plug-in:" << iPluginName);
        return false;
    }

    iThread = new ClientThread();
    if (iThread == 0)
    {
        LOG_WARNING("Failed to create client thread");
        return false;
    }

    // Pass connectivity state change signal to the plug-in.
    connect(this, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)),
        iPlugin, SLOT(connectivityStateChanged(Sync::ConnectivityType, bool)));

    // Connect signals from the plug-in.

    connect(iPlugin, SIGNAL(transferProgress(const QString &, Sync::TransferDatabase, Sync::TransferType, const QString &, int)),
        this, SLOT(onTransferProgress(const QString &, Sync::TransferDatabase, Sync::TransferType, const QString &, int)));

    connect(iPlugin, SIGNAL(error(const QString &, const QString &, int)),
        this, SLOT(onError(const QString &, const QString &, int)));

    connect(iPlugin, SIGNAL(success(const QString &, const QString &)),
        this, SLOT(onSuccess(const QString &, const QString &)));

    connect(iPlugin, SIGNAL(accquiredStorage(const QString &)),
        this, SLOT(onStorageAccquired(const QString &)));

    connect(iPlugin,SIGNAL(syncProgressDetail(const QString &,int)),
    		this ,SLOT(onSyncProgressDetail(const QString &,int)));

    // Connect signals from the thread.
    connect(iThread, SIGNAL(initError(const QString &, const QString &, int)),
        this, SLOT(onError(const QString &, const QString &, int)));

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    connect(iThread, SIGNAL(terminated()), this, SLOT(onThreadExit()));
#endif

    connect(iThread, SIGNAL(finished()), this, SLOT(onThreadExit()));

    iInitialized = true;

    return true;
}

bool ClientPluginRunner::start()
{
    FUNCTION_CALL_TRACE;

    bool rv = false;
    if (iInitialized && iThread != 0)
    {
        rv = iThread->startThread(iPlugin);
    }

    return rv;
}

void ClientPluginRunner::stop()
{
    FUNCTION_CALL_TRACE;

    if (iThread != 0)
    {
        iThread->stopThread();
        iThread->wait();
    }
}

void ClientPluginRunner::abort(Sync::SyncStatus aStatus)
{
    FUNCTION_CALL_TRACE;

    if (iPlugin != 0)
    {
        iPlugin->abortSync(aStatus);
    }
}

SyncPluginBase *ClientPluginRunner::plugin()
{
    FUNCTION_CALL_TRACE;

    return iPlugin;
}

SyncResults ClientPluginRunner::syncResults()
{
    FUNCTION_CALL_TRACE;

    if (iPlugin != 0)
    {
        return iPlugin->getSyncResults();
    }
    else
    {
        return SyncResults();
    }
}

bool ClientPluginRunner::cleanUp()
{
    FUNCTION_CALL_TRACE;

    bool retval = false;
    if (iPlugin != 0)
    {
        retval = iPlugin->cleanUp();
    }
    return retval;
}

void ClientPluginRunner::onTransferProgress(const QString &aProfileName,
    Sync::TransferDatabase aDatabase, Sync::TransferType aType,
    const QString &aMimeType, int aCommittedItems)
{
    FUNCTION_CALL_TRACE;

    emit transferProgress(aProfileName, aDatabase, aType, aMimeType, aCommittedItems);
}

void ClientPluginRunner::onError(const QString &aProfileName,
                                 const QString &aMessage, int aErrorCode)
{
    FUNCTION_CALL_TRACE;

    emit error(aProfileName, aMessage, aErrorCode);
    stop();
}

void ClientPluginRunner::onSuccess(const QString &aProfileName,
                                   const QString &aMessage)
{
    FUNCTION_CALL_TRACE;

    emit success(aProfileName, aMessage);
    stop();

}

void ClientPluginRunner::onStorageAccquired(const QString &aMimeType )
{
    FUNCTION_CALL_TRACE;

    emit storageAccquired(aMimeType);
}

void ClientPluginRunner::onSyncProgressDetail(const QString &aProfileName,int aProgressDetail)
{
	FUNCTION_CALL_TRACE;

	emit syncProgressDetail(aProfileName,aProgressDetail);
}


void ClientPluginRunner::onThreadExit()
{
    FUNCTION_CALL_TRACE;

    emit done();
}

