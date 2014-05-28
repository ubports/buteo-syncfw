/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
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
#include <SyncProfile.h>
#include <ProfileManager.h>
#include <LogMacros.h>
#include <SyncCommonDefs.h>

using namespace Buteo;

PluginServiceObj::PluginServiceObj(QObject *parent) :
    QObject(parent), iProfile(0), iPlugin(0)
{
}

PluginServiceObj::PluginServiceObj( QString aProfileName, QString aPluginName, QObject *parent) :
    QObject(parent), iProfile(0), iPlugin(0)
{
    ProfileManager pm;
    SyncProfile *syncProfile = pm.syncProfile( aProfileName );
    if( syncProfile ) {
        iProfile = syncProfile;
    }

    // Create the plugin (client or server)
    iPlugin = new CLASSNAME( iPluginName, *iProfile, NULL );
}

PluginServiceObj::~PluginServiceObj()
{
    if( iProfile ) {
        delete iProfile;
        iProfile = 0;
    }

    if( iPlugin ) {
        delete iPlugin;
        iPlugin = 0;
    }
}

void PluginServiceObj::setPluginParams(const QString &aPluginName,
                                       const QString &aProfileName)
{
    iPluginName = aPluginName;
    ProfileManager pm;
    SyncProfile *syncProfile = pm.syncProfile(aProfileName);
    if( syncProfile ) {
        iProfile = syncProfile;
    }
}

bool PluginServiceObj::init()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->init();
}

bool PluginServiceObj::uninit()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->uninit();
}

bool PluginServiceObj::startSync()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->startSync();
}

void PluginServiceObj::abortSync(uchar aStatus)
{
    FUNCTION_CALL_TRACE;

    iPlugin->abortSync( static_cast<Sync::SyncStatus>(aStatus) );
}

bool PluginServiceObj::cleanUp()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->cleanUp();
}

void PluginServiceObj::connectivityStateChanged(int aType, bool aState)
{
    FUNCTION_CALL_TRACE;

    iPlugin->connectivityStateChanged( static_cast<Sync::ConnectivityType>(aType), aState );
}

QString PluginServiceObj::getPluginName()
{
    FUNCTION_CALL_TRACE;

    return iPluginName;
}

QString PluginServiceObj::getProfileName()
{
    FUNCTION_CALL_TRACE;

    return iProfile->name();
}

QString PluginServiceObj::getSyncResults()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->getSyncResults().toString();
}

QString PluginServiceObj::profile()
{
    FUNCTION_CALL_TRACE;

    return iProfile->toString();
}

#ifndef CLIENT_PLUGIN
void PluginServiceObj::resume()
{
    FUNCTION_CALL_TRACE;

    iPlugin->resume();
}

bool PluginServiceObj::startListen()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->startListen();
}

void PluginServiceObj::stopListen()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->stopListen();
}

void PluginServiceObj::suspend()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->suspend();
}
#endif

void PluginServiceObj::exitWithSyncSuccess( QString aProfileName,
                                            QString aState )
{
}

void PluginServiceObj::exitWithSyncFailed( QString aProfileName,
                                           QString aMessage,
                                           int aErrorCode )
{
}
