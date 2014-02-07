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

#include <QDomDocument>
#include "OOPClientPlugin.h"
#include "LogMacros.h"

using namespace Buteo;

OOPClientPlugin::OOPClientPlugin(const QString& aPluginName,
                                 const SyncProfile& aProfile,
                                 PluginCbInterface* aCbInterface,
                                 QProcess &aProcess ) : 
    ClientPlugin( aPluginName, aProfile, aCbInterface )
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

    connect(iOopPluginIface,SIGNAL(syncProgressDetail(const QString &,int)),
    		this ,SIGNAL(syncProgressDetail(const QString &,int)));

    connect(&aProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SIGNAL(processError(QProcess::ProcessError)));

    connect(&aProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SIGNAL(processFinished(int,QProcess::ExitStatus)));
}

OOPClientPlugin::~OOPClientPlugin()
{
    if( iOopPluginIface ) {
        delete iOopPluginIface;
        iOopPluginIface = 0;
    }
}

bool OOPClientPlugin::init()
{
    FUNCTION_CALL_TRACE;
    QDBusReply<bool> reply = iOopPluginIface->init();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for init from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPClientPlugin::uninit()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->uninit();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for uninit from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPClientPlugin::startSync()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->startSync();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for startSync from plugin" );
        return false;
    }

    return reply.value();
}

void OOPClientPlugin::abortSync( Sync::SyncStatus aStatus )
{
    FUNCTION_CALL_TRACE;

    QDBusReply<void> reply = iOopPluginIface->abortSync( (uchar)aStatus );
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for connectivityStateChanged from plugin" );
}

bool OOPClientPlugin::cleanUp()
{
    FUNCTION_CALL_TRACE;

    QDBusReply<bool> reply = iOopPluginIface->cleanUp();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for cleanUp from plugin" );
        return false;
    }

    return reply.value();
}

SyncResults OOPClientPlugin::getSyncResults() const
{
    FUNCTION_CALL_TRACE;

    SyncResults errorSyncResult( QDateTime(),
                            SyncResults::SYNC_RESULT_INVALID,
                            SyncResults::SYNC_RESULT_INVALID );
    QDBusReply<QString> reply = iOopPluginIface->getSyncResults();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for getSyncResults from plugin" );
        return errorSyncResult;
    }

    QString resultAsXml = reply.value();
    QDomDocument doc;
    if( doc.setContent(resultAsXml, true) ) {
        SyncResults syncResult( doc.documentElement() );
        return syncResult;
    } else {
        LOG_CRITICAL( "Invalid sync results returned from plugin" );
        return errorSyncResult;
    }
}

void OOPClientPlugin::connectivityStateChanged( Sync::ConnectivityType aType,
                                                bool aState )
{
    FUNCTION_CALL_TRACE;

    QDBusReply<void> reply = iOopPluginIface->connectivityStateChanged( aType, aState );
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for connectivityStateChanged from plugin" );
}
