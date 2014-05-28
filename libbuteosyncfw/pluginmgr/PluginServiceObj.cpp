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
#include <ProfileManager.h>
#include <LogMacros.h>
#include <SyncCommonDefs.h>

using namespace Buteo;

PluginServiceObj::PluginServiceObj( QString aProfileName, QString aPluginName, QObject *parent) :
    QObject(parent), iPlugin(0), iProfileName(aProfileName), iPluginName(aPluginName)
{
}

PluginServiceObj::~PluginServiceObj()
{
    if( iPlugin ) {
        delete iPlugin;
        iPlugin = 0;
    }
}

bool PluginServiceObj::init()
{
    FUNCTION_CALL_TRACE;

    ProfileManager pm;
#ifdef CLIENT_PLUGIN
    SyncProfile *syncProfile = pm.syncProfile( iProfileName );
    if( !syncProfile ) {
        LOG_WARNING( "Profile " << iProfileName << " does not exist" );
        return false;
    }

    // Create the plugin (client or server)
    iPlugin = new CLASSNAME( iPluginName, *syncProfile, &iPluginCb );
#else
    Profile *profile = pm.profile( iProfileName, Profile::TYPE_SERVER );
    if( !profile || !profile->isValid() ) {
        LOG_WARNING( "Profile " << iProfileName << " does not exist" );
        return false;
    } else {
        pm.expand( *profile );
    }

    // Create the plugin (client or server)
    iPlugin = new CLASSNAME( iPluginName, *profile, &iPluginCb );

    // Server signals
    QObject::connect(iPlugin, SIGNAL(newSession(const QString&)),
                     this, SIGNAL(newSession(const QString&)));
#endif
    // Chain the signals
    QObject::connect(iPlugin, SIGNAL(transferProgress(const QString&, int, int, const QString&, int)),
                     this, SIGNAL(transferProgress(const QString&, int, int, const QString&, int)));
    QObject::connect(iPlugin, SIGNAL(error(const QString&, const QString&, int)),
                     this, SIGNAL(error(const QString&, const QString&, int)));
    QObject::connect(iPlugin, SIGNAL(success(const QString&, const QString&)),
                     this, SIGNAL(success(const QString&, const QString&)));
    QObject::connect(iPlugin, SIGNAL(accquiredStorage(const QString&)),
                     this, SIGNAL(accquiredStorage(const QString&)));
    QObject::connect(iPlugin, SIGNAL(syncProgressDetail(const QString&, int)),
                     this, SIGNAL(syncProgressDetail(const QString&, int)));

    return iPlugin->init();
}

bool PluginServiceObj::uninit()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->uninit();
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

QString PluginServiceObj::getSyncResults()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->getSyncResults().toString();
}

#ifdef CLIENT_PLUGIN
bool PluginServiceObj::startSync()
{
    FUNCTION_CALL_TRACE;

    return iPlugin->startSync();
}

#else
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
