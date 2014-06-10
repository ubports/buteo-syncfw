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
#include "OOPServerPlugin.h"
#include "LogMacros.h"

using namespace Buteo;

OOPServerPlugin::OOPServerPlugin( const QString& aPluginName,
                                  const Profile& aProfile,
                                  PluginCbInterface* aCbInterface,
                                  QProcess& aProcess ) :
    ServerPlugin( aPluginName, aProfile, aCbInterface )
    
{
    FUNCTION_CALL_TRACE;

    // Initialise dbus for client
    iOopPluginIface = new ButeoPluginIface( DBUS_SERVICE_NAME_PREFIX + aProfile.name(),
                                         DBUS_SERVICE_OBJ_PATH,
                                         QDBusConnection::sessionBus()
                                       );

    // Chain the signals received over dbus
    connect(iOopPluginIface, SIGNAL(transferProgress(const QString &, int, int, const QString &, int)),
        this, SIGNAL(transferProgress(const QString &, int, int, const QString &, int)));

    connect(iOopPluginIface, SIGNAL(error(const QString &, const QString &, int)),
        this, SIGNAL(error(const QString &, const QString &, int)));

    connect(iOopPluginIface, SIGNAL(success(const QString &, const QString &)),
        this, SIGNAL(success(const QString &, const QString &)));

    connect(iOopPluginIface, SIGNAL(accquiredStorage(const QString &)),
        this, SIGNAL(accquiredStorage(const QString &)));

    connect(iOopPluginIface, SIGNAL(newSession(const QString&)),
            this, SIGNAL(newSession(const QString&)));

    connect(&aProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SIGNAL(processError(QProcess::ProcessError)));

    connect(&aProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SIGNAL(processFinished(int,QProcess::ExitStatus)));
}

OOPServerPlugin::~OOPServerPlugin()
{
    FUNCTION_CALL_TRACE;

    if( iOopPluginIface ) {
        delete iOopPluginIface;
        iOopPluginIface = 0;
    }
}

bool OOPServerPlugin::init()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->init();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for init from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPServerPlugin::uninit()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->startSync();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for startSync from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPServerPlugin::startListen()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->startListen();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for startListen from plugin" );
        return false;
    }

    return reply.value();
}

void OOPServerPlugin::stopListen()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<void> reply = iOopPluginIface->stopListen();
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for stopListen from plugin" );
}

void OOPServerPlugin::suspend()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<void> reply = iOopPluginIface->suspend();
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for suspend from plugin" );
}

void OOPServerPlugin::resume()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<void> reply = iOopPluginIface->resume();
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for resume from plugin" );
}

bool OOPServerPlugin::cleanUp()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->cleanUp();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for cleanUp from plugin" );
        return false;
    }

    return reply.value();
}

void OOPServerPlugin::connectivityStateChanged( Sync::ConnectivityType aType,
                                                bool aState )
{
    FUNCTION_CALL_TRACE;

    QDBusReply<void> reply = iOopPluginIface->connectivityStateChanged(aType, aState);
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for connectivityStateChanged from plugin" );
}
