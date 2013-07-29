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

#include "ServerPluginRunner.h"
#include "ServerThread.h"
#include "ServerActivator.h"
#include "ServerPlugin.h"
#include "LogMacros.h"
#include "PluginManager.h"

using namespace Buteo;

ServerPluginRunner::ServerPluginRunner(const QString &aPluginName,
    Profile *aProfile, PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf,
    ServerActivator *aServerActivator, QObject *aParent)
:   PluginRunner(PLUGIN_SERVER, aPluginName, aPluginMgr, aPluginCbIf, aParent),
    iProfile(aProfile),
    iPlugin(0),
    iThread(0),
    iServerActivator(aServerActivator)
{
    FUNCTION_CALL_TRACE;
}

ServerPluginRunner::~ServerPluginRunner()
{
    FUNCTION_CALL_TRACE;

    disconnect();

    if (iPlugin != 0 && iPluginMgr != 0)
    {
        iPluginMgr->destroyServer(iPlugin);
        iPlugin = 0;
    }

    delete iThread;
    iThread = 0;

    delete iProfile;
    iProfile = 0;
}

bool ServerPluginRunner::init()
{
    FUNCTION_CALL_TRACE;

    if (iInitialized)
        return true;

    if (iPluginMgr == 0 || iPluginCbIf == 0 || iProfile == 0)
    {
        LOG_WARNING("Invalid members, failed to initialize");
        return false;
    }

    iPlugin = iPluginMgr->createServer(iPluginName, *iProfile, iPluginCbIf);
    if (iPlugin == 0)
    {
        LOG_WARNING("Failed to create server plug-in:" << iPluginName);
        return false;
    }

    iThread = new ServerThread();
    if (iThread == 0)
    {
        LOG_WARNING("Failed to create server thread");
        return false;
    }

    // Pass connectivity state change signal to the plug-in.
    connect(this, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)),
        iPlugin, SLOT(connectivityStateChanged(Sync::ConnectivityType, bool)));

    connect(iPlugin, SIGNAL(newSession(const QString &)),
        this, SLOT(onNewSession(const QString &)));

    // Connect signals from the plug-in.

    connect(iPlugin, SIGNAL(transferProgress(const QString &, Sync::TransferDatabase, Sync::TransferType, const QString &,int)),
        this, SLOT(onTransferProgress(const QString &, Sync::TransferDatabase, Sync::TransferType, const QString &,int)));

    connect(iPlugin, SIGNAL(error(const QString &, const QString &, int)),
        this, SLOT(onError(const QString &, const QString &, int)));

    connect(iPlugin, SIGNAL(success(const QString &, const QString &)),
        this, SLOT(onSuccess(const QString &, const QString &)));
    
    connect(iPlugin, SIGNAL(accquiredStorage(const QString &)),
        this, SLOT(onStorageAccquired(const QString &)));

    connect(iPlugin,SIGNAL(syncProgressDetail(const QString &,int)),
            this ,SIGNAL(syncProgressDetail(const QString &,int)));

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

bool ServerPluginRunner::start()
{
    FUNCTION_CALL_TRACE;

    bool rv = false;
    if (iInitialized && iThread != 0)
    {
        rv = iThread->startThread(iPlugin);
    }

    return rv;
}

void ServerPluginRunner::stop()
{
    FUNCTION_CALL_TRACE;

    // Disconnect all signals from this object to the plug-in.
    disconnect(this, 0, iPlugin, 0);

    if (iThread != 0)
    {
        iThread->stopThread();
        iThread->wait();
    }
}

void ServerPluginRunner::abort(Sync::SyncStatus aStatus)
{
    FUNCTION_CALL_TRACE;

    if (iPlugin != 0)
    {
        iPlugin->abortSync(aStatus);
    }
}

void ServerPluginRunner::suspend()
{
    FUNCTION_CALL_TRACE;

    if (iPlugin != 0)
    {
        iPlugin->suspend();
    }
}

void ServerPluginRunner::resume()
{
    FUNCTION_CALL_TRACE;

    if (iPlugin != 0)
    {
        iPlugin->resume();
    }
}

SyncPluginBase *ServerPluginRunner::plugin()
{
    FUNCTION_CALL_TRACE;

    return iPlugin;
}

SyncResults ServerPluginRunner::syncResults()
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

bool ServerPluginRunner::cleanUp()
{
    FUNCTION_CALL_TRACE;

    bool retval = false ;
    if (iPlugin != 0)
    {
        retval = iPlugin->cleanUp();
    }
    return retval;
}

void ServerPluginRunner::onNewSession(const QString &aDestination)
{
    // Add reference to the server plug-in, so that the plug-in
    // will not be stopped when there is a session running.
    if (iServerActivator != 0)
    {
        iServerActivator->addRef(plugin()->getProfileName());
    }

    emit newSession(aDestination);
}

void ServerPluginRunner::onTransferProgress(const QString &aProfileName,
    Sync::TransferDatabase aDatabase, Sync::TransferType aType,
    const QString &aMimeType, int aCommittedItems)
{
    FUNCTION_CALL_TRACE;

    emit transferProgress(aProfileName, aDatabase, aType, aMimeType, aCommittedItems);
}

void ServerPluginRunner::onError(const QString &aProfileName,
                                 const QString &aMessage, int aErrorCode)
{
    FUNCTION_CALL_TRACE;

    emit error(aProfileName, aMessage, aErrorCode);

    onSessionDone();
}

void ServerPluginRunner::onSuccess(const QString &aProfileName,
                                   const QString &aMessage)
{
    FUNCTION_CALL_TRACE;

    emit success(aProfileName, aMessage);

    onSessionDone();
}

void ServerPluginRunner::onStorageAccquired(const QString &aMimeType )
{
    FUNCTION_CALL_TRACE;

    emit storageAccquired(aMimeType);
}

void ServerPluginRunner::onThreadExit()
{
    FUNCTION_CALL_TRACE;

    emit done();
}

void ServerPluginRunner::onSessionDone()
{
    FUNCTION_CALL_TRACE;

    // Remove reference to the server plug-in. This may result in stopping
    // the server plug-in, if it doesn't need to be active any more.
    if (iServerActivator != 0)
    {
        iServerActivator->removeRef(plugin()->getProfileName());
    }
}
