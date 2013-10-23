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

#include "OOPClientPlugin.h"
using namespace Buteo;

#include "LogMacros.h"

OOPClientPlugin::OOPClientPlugin( const QString& aPluginName,
                                  const SyncProfile& aProfile,
                                  PluginCbInterface* aCbInterface) : 
    ClientPlugin( aPluginName, aProfile, aCbInterface )
{
    FUNCTION_CALL_TRACE;
}

OOPClientPlugin::~OOPClientPlugin()
{
}

bool OOPClientPlugin::init()
{
    FUNCTION_CALL_TRACE;
}

bool OOPClientPlugin::uninit()
{
    FUNCTION_CALL_TRACE;
}

bool OOPClientPlugin::startSync()
{
    FUNCTION_CALL_TRACE;
}

void OOPClientPlugin::abortSync( Sync::SyncStatus aStatus )
{
    FUNCTION_CALL_TRACE;
}

bool OOPClientPlugin::cleanUp()
{
    FUNCTION_CALL_TRACE;
}

SyncResults OOPClientPlugin::getSyncResults() const
{
    FUNCTION_CALL_TRACE;
}

void OOPClientPlugin::connectivityStateChanged( Sync::ConnectivityType aType,
                                                bool aState )
{
    FUNCTION_CALL_TRACE;
}
