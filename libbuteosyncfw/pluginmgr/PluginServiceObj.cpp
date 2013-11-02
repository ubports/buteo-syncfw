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
    return true;
}

bool PluginServiceObj::uninit()
{
    return true;
}

void PluginServiceObj::startSync()
{
}

void PluginServiceObj::abortSync(uchar aStatus)
{
}

bool PluginServiceObj::cleanUp()
{
}

void PluginServiceObj::connectivityStateChanged(int aType, int aState)
{
}

QString PluginServiceObj::getPluginName()
{
}

QString PluginServiceObj::getProfileName()
{
}

QString PluginServiceObj::getSyncResults()
{
}

QString PluginServiceObj::profile()
{
}

void PluginServiceObj::resume()
{
}

bool PluginServiceObj::startListen()
{
}

void PluginServiceObj::stopListen()
{
}

void PluginServiceObj::suspend()
{
}

void PluginServiceObj::exitWithSyncSuccess( QString aProfileName,
                                            QString aState )
{
}

void PluginServiceObj::exitWithSyncFailed( QString aProfileName,
                                           QString aMessage,
                                           int aErrorCode )
{
}
