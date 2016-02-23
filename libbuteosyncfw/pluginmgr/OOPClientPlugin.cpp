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

#include <QRegExp>

using namespace Buteo;

OOPClientPlugin::OOPClientPlugin(const QString& aPluginName,
                                 const SyncProfile& aProfile,
                                 PluginCbInterface* aCbInterface,
                                 QProcess &aProcess ) : 
    ClientPlugin( aPluginName, aProfile, aCbInterface ), iDone( false )
{
    FUNCTION_CALL_TRACE;

    // randomly-generated profile names cannot be registered
    // as dbus service paths due to being purely numeric.
    QString profileName = aProfile.name();
    int numericIdx = profileName.indexOf(QRegExp("[0123456789]"));
    QString servicePath = numericIdx == 0
                        ? QString(QLatin1String("%1%2%3"))
                              .arg(DBUS_SERVICE_NAME_PREFIX)
                              .arg("profile-")
                              .arg(profileName)
                        : QString(QLatin1String("%1%2"))
                              .arg(DBUS_SERVICE_NAME_PREFIX)
                              .arg(profileName);

    // Initialise dbus for client
    iOopPluginIface = new ButeoPluginIface( servicePath,
                                         DBUS_SERVICE_OBJ_PATH,
                                         QDBusConnection::sessionBus()
                                       );
    iOopPluginIface->setTimeout(60000); // one minute.

    // Chain the signals received over dbus
    connect(iOopPluginIface, SIGNAL(transferProgress(const QString &,
            Sync::TransferDatabase, Sync::TransferType, const QString &, int)),
        this, SIGNAL(transferProgress(const QString &,
            Sync::TransferDatabase, Sync::TransferType, const QString &, int)));

    connect(iOopPluginIface, SIGNAL(error(QString,QString,int)),
        this, SLOT(onError(QString,QString,int)));

    connect(iOopPluginIface, SIGNAL(success(QString,QString)),
        this, SLOT(onSuccess(QString,QString)));

    connect(iOopPluginIface, SIGNAL(accquiredStorage(const QString &)),
        this, SIGNAL(accquiredStorage(const QString &)));

    connect(iOopPluginIface,SIGNAL(syncProgressDetail(const QString &,int)),
            this ,SIGNAL(syncProgressDetail(const QString &,int)));

    // Handle the signals from the process
    connect(&aProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onProcessError(QProcess::ProcessError)));

    connect(&aProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onProcessFinished(int,QProcess::ExitStatus)));
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
    QDBusPendingReply<bool> reply = iOopPluginIface->init();
    reply.waitForFinished();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for init from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPClientPlugin::uninit()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->uninit();
    reply.waitForFinished();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for uninit from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPClientPlugin::startSync()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->startSync();
    reply.waitForFinished();
    if( !reply.isValid() ) {
        LOG_WARNING( "Invalid reply for startSync from plugin" );
        return false;
    }

    return reply.value();
}

void OOPClientPlugin::abortSync( Sync::SyncStatus aStatus )
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<void> reply = iOopPluginIface->abortSync( (uchar)aStatus );
    reply.waitForFinished();
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for abortSync from plugin" );
}

bool OOPClientPlugin::cleanUp()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->cleanUp();
    reply.waitForFinished();
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
    QDBusPendingReply<QString> reply = iOopPluginIface->getSyncResults();
    reply.waitForFinished();
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

    QDBusPendingReply<void> reply = iOopPluginIface->connectivityStateChanged( aType, aState );
    reply.waitForFinished();
    if( !reply.isValid() )
        LOG_WARNING( "Invalid reply for connectivityStateChanged from plugin" );
}

void OOPClientPlugin::onProcessError( QProcess::ProcessError error )
{
    if( !iDone ) {
        onError( iProfile.name(),
                 "Plugin process error:" + QString::number(error),
                 Sync::SYNC_PLUGIN_ERROR );
    }
}

void OOPClientPlugin::onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( !iDone ) {
        if( (exitCode != 0) || (exitStatus != QProcess::NormalExit) ) {
            onError( iProfile.name(),
                    "Plugin process exited with error code " +
                     QString::number(exitCode) + " and status " +
                     QString::number(exitStatus),
                    Sync::SYNC_PLUGIN_ERROR );
        } else {
            onError( iProfile.name(),
                    "Plugin process exited unexpectedly",
                    Sync::SYNC_PLUGIN_ERROR );
        }
    }
}

void OOPClientPlugin::onError(QString aProfileName, QString aMessage, int aErrorCode)
{
    if ( !iDone ) {
        iDone = true;
        emit error(aProfileName, aMessage, aErrorCode);
    }
}

void OOPClientPlugin::onSuccess(QString aProfileName, QString aMessage)
{
    if ( !iDone ) {
        iDone = true;
        emit success(aProfileName, aMessage);
    }
}
