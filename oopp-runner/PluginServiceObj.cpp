/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 - 2021 Jolla Ltd.
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
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
*/
#include "PluginServiceObj.h"

#include <SyncResults.h>
#include <ProfileManager.h>
#include <LogMacros.h>
#include <SyncCommonDefs.h>
#include <ClientPlugin.h>
#include <ServerPlugin.h>

#include <QPluginLoader>
#include <QFileInfo>

using namespace Buteo;

PluginServiceObj::PluginServiceObj(const QString &aPluginName,
                                   const QString &aProfileName,
                                   const QString &aPluginFilePath,
                                   QObject *parent)
    : QObject(parent)
    , iPluginCb(new Buteo::PluginCbImpl)
    , iPluginName(aPluginName)
    , iProfileName(aProfileName)
    , iPluginFilePath(aPluginFilePath)
{
}

PluginServiceObj::~PluginServiceObj()
{
    if (iPlugin) {
        delete iPlugin;
        iPlugin = nullptr;
    }

    delete iPluginCb;
}

SyncPluginBase *PluginServiceObj::initializePlugin()
{
    if (!iPluginLoader) {
        iPluginLoader = new QPluginLoader(iPluginFilePath, this);
    }

    iSyncPluginLoader = qobject_cast<SyncPluginLoader *>(iPluginLoader->instance());
    if (!iSyncPluginLoader) {
        LOG_WARNING("Unable to load SyncPluginLoader " << iPluginName
                    << " from path " << iPluginFilePath);
        return nullptr;
    }

    ProfileManager pm;
    const QString pluginBaseName = QFileInfo(iPluginFilePath).baseName();

    if (pluginBaseName.endsWith(QStringLiteral("-client"))) {
        SyncProfile *syncProfile = pm.syncProfile(iProfileName);
        if (!syncProfile) {
            LOG_WARNING("Profile " << iProfileName << " does not exist");
            return nullptr;
        }

        // Create the plugin (client)
        return iSyncPluginLoader->createClientPlugin(iPluginName, *syncProfile, iPluginCb);

    } else if (pluginBaseName.endsWith(QStringLiteral("-server"))) {
        Profile *profile = pm.profile(iProfileName, Profile::TYPE_SERVER);
        if (!profile || !profile->isValid()) {
            LOG_WARNING("Profile " << iProfileName << " does not exist");
            return nullptr;
        } else {
            pm.expand(*profile);
        }

        // Create the plugin (server)
        return iSyncPluginLoader->createServerPlugin(iPluginName, *profile, iPluginCb);

    } else {
        LOG_WARNING("Plugin is neither -client nor -server type:" << iPluginName);
        return nullptr;
    }
}

bool PluginServiceObj::init()
{
    FUNCTION_CALL_TRACE;

    iPlugin = initializePlugin();
    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::init(): unable to initialize plugin" );
        return false;
    }

    if (ServerPlugin *serverPlugin = qobject_cast<ServerPlugin*>(iPlugin)) {
        // Server signals
        connect(serverPlugin, &ServerPlugin::newSession,
                this, &PluginServiceObj::newSession);
    }

    // Chain the signals
    connect(iPlugin, &SyncPluginBase::transferProgress,
            this, &PluginServiceObj::transferProgress);
    connect(iPlugin, &SyncPluginBase::error,
            this, &PluginServiceObj::error);
    connect(iPlugin, &SyncPluginBase::success,
            this, &PluginServiceObj::success);
    connect(iPlugin, &SyncPluginBase::accquiredStorage,
            this, &PluginServiceObj::accquiredStorage);
    connect(iPlugin, &SyncPluginBase::syncProgressDetail,
            this, &PluginServiceObj::syncProgressDetail);

    return iPlugin->init();
}

bool PluginServiceObj::uninit()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::uninit(): called on uninitialized plugin" );
        return true;
    }

    if (iPlugin->uninit()) {
        delete iPlugin;
        iPlugin = nullptr;
        iPluginLoader->unload();
        return true;
    }

    return false;
}

void PluginServiceObj::abortSync(uchar aStatus)
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::abortSync(): called on uninitialized plugin" );
        return;
    }
    iPlugin->abortSync( static_cast<Sync::SyncStatus>(aStatus) );
}

bool PluginServiceObj::cleanUp()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        initializePlugin();
        if (!iPlugin) {
            LOG_WARNING( "PluginServiceObj::cleanUp(): unable to initialize plugin" );
            return false;
        }
    }

    return iPlugin->cleanUp();
}

void PluginServiceObj::connectivityStateChanged(int aType, bool aState)
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::connectivityStateChanged(): called on uninitialized plugin" );
        return;
    }
    iPlugin->connectivityStateChanged( static_cast<Sync::ConnectivityType>(aType), aState );
}

QString PluginServiceObj::getSyncResults()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::getSyncResults(): called on uninitialized plugin" );
        return QString();
    }
    return iPlugin->getSyncResults().toString();
}

bool PluginServiceObj::startSync()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::startSync(): called on uninitialized plugin" );
        return false;
    }

    if (ClientPlugin *clientPlugin = qobject_cast<ClientPlugin*>(iPlugin)) {
        return clientPlugin->startSync();
    } else {
        LOG_WARNING( "PluginServiceObj::startSync(): client plugin unavailable" );
        return false;
    }
}

void PluginServiceObj::resume()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::resume(): called on uninitialized plugin" );
        return;
    }

    if (ServerPlugin *serverPlugin = qobject_cast<ServerPlugin*>(iPlugin)) {
        serverPlugin->resume();
    } else {
        LOG_WARNING( "PluginServiceObj::resume(): server plugin unavailable" );
    }
}

bool PluginServiceObj::startListen()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::startListen(): called on uninitialized plugin" );
        return false;
    }

    if (ServerPlugin *serverPlugin = qobject_cast<ServerPlugin*>(iPlugin)) {
        return serverPlugin->startListen();
    } else {
        LOG_WARNING( "PluginServiceObj::startListen(): server plugin unavailable" );
        return false;
    }
}

void PluginServiceObj::stopListen()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::stopListen(): called on uninitialized plugin" );
        return;
    }

    if (ServerPlugin *serverPlugin = qobject_cast<ServerPlugin*>(iPlugin)) {
        serverPlugin->stopListen();
    } else {
        LOG_WARNING( "PluginServiceObj::stopListen(): server plugin unavailable" );
    }
}

void PluginServiceObj::suspend()
{
    FUNCTION_CALL_TRACE;

    if (!iPlugin) {
        LOG_WARNING( "PluginServiceObj::suspend(): called on uninitialized plugin" );
        return;
    }

    if (ServerPlugin *serverPlugin = qobject_cast<ServerPlugin*>(iPlugin)) {
        serverPlugin->suspend();
    } else {
        LOG_WARNING( "PluginServiceObj::suspend(): server plugin unavailable" );
    }
}

