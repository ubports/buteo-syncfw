/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd.
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

#include <QRegExp>

using namespace Buteo;

OOPServerPlugin::OOPServerPlugin(const QString &aPluginName,
                                 const Profile &aProfile,
                                 PluginCbInterface *aCbInterface,
                                 QProcess &aProcess)
    : ServerPlugin(aPluginName, aProfile, aCbInterface), iDone(false)
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

    // Initialise dbus for server
    iOopPluginIface = new ButeoPluginIface(servicePath,
                                           DBUS_SERVICE_OBJ_PATH,
                                           QDBusConnection::sessionBus());
    iOopPluginIface->setTimeout(60000); // one minute.

    // Chain the signals received over dbus
    connect(iOopPluginIface, SIGNAL(transferProgress(const QString &, Sync::TransferDatabase, Sync::TransferType,
                                                     const QString &, int)),
            this, SIGNAL(transferProgress(const QString &, Sync::TransferDatabase, Sync::TransferType, const QString &, int)));

    connect(iOopPluginIface, SIGNAL(error(QString, QString, int)),
            this, SLOT(onError(QString, QString, int)));

    connect(iOopPluginIface, SIGNAL(success(QString, QString)),
            this, SLOT(onSuccess(QString, QString)));

    connect(iOopPluginIface, SIGNAL(accquiredStorage(const QString &)),
            this, SIGNAL(accquiredStorage(const QString &)));

    connect(iOopPluginIface, SIGNAL(newSession(const QString &)),
            this, SIGNAL(newSession(const QString &)));

    // Handle the signals from the process
    connect(&aProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onProcessError(QProcess::ProcessError)));

    connect(&aProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
}

OOPServerPlugin::~OOPServerPlugin()
{
    FUNCTION_CALL_TRACE;
    delete iOopPluginIface;
    iOopPluginIface = 0;
}

bool OOPServerPlugin::init()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->init();
    reply.waitForFinished();
    if (!reply.isValid()) {
        LOG_WARNING( "Invalid reply for init from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPServerPlugin::uninit()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->uninit();
    reply.waitForFinished();
    if (!reply.isValid()) {
        LOG_WARNING( "Invalid reply for startSync from plugin" );
        return false;
    }

    return reply.value();
}

bool OOPServerPlugin::startListen()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->startListen();
    reply.waitForFinished();
    if (!reply.isValid()) {
        LOG_WARNING( "Invalid reply for startListen from plugin" );
        return false;
    }

    return reply.value();
}

void OOPServerPlugin::stopListen()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->stopListen();
    reply.waitForFinished();
    if (!reply.isValid())
        LOG_WARNING( "Invalid reply for stopListen from plugin" );
}

void OOPServerPlugin::suspend()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->suspend();
    reply.waitForFinished();
    if (!reply.isValid())
        LOG_WARNING( "Invalid reply for suspend from plugin" );
}

void OOPServerPlugin::resume()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->resume();
    reply.waitForFinished();
    if (!reply.isValid())
        LOG_WARNING( "Invalid reply for resume from plugin" );
}

bool OOPServerPlugin::cleanUp()
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<bool> reply = iOopPluginIface->cleanUp();
    reply.waitForFinished();
    if (!reply.isValid()) {
        LOG_WARNING( "Invalid reply for cleanUp from plugin" );
        return false;
    }

    return reply.value();
}

void OOPServerPlugin::connectivityStateChanged(Sync::ConnectivityType aType, bool aState)
{
    FUNCTION_CALL_TRACE;

    QDBusPendingReply<void> reply = iOopPluginIface->connectivityStateChanged(aType, aState);
    reply.waitForFinished();
    if (!reply.isValid())
        LOG_WARNING( "Invalid reply for connectivityStateChanged from plugin" );
}

void OOPServerPlugin::onProcessError(QProcess::ProcessError error)
{
    if (!iDone) {
        onError(iProfile.name(),
                "Plugin process error:" + QString::number(error),
                SyncResults::PLUGIN_ERROR);
    }
}

void OOPServerPlugin::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!iDone) {
        if ((exitCode != 0) || (exitStatus != QProcess::NormalExit)) {
            onError(iProfile.name(),
                    "Plugin process exited with error code " +
                    QString::number(exitCode) + " and status " +
                    QString::number(exitStatus),
                    SyncResults::PLUGIN_ERROR);
        } else {
            onError(iProfile.name(),
                    "Plugin process exited unexpectedly",
                    SyncResults::PLUGIN_ERROR);
        }
    }
}

void OOPServerPlugin::onError(QString aProfileName, QString aMessage, int aErrorCode)
{
    if (!iDone) {
        iDone = true;
        emit error(aProfileName, aMessage, aErrorCode);
    }
}

void OOPServerPlugin::onSuccess(QString aProfileName, QString aMessage)
{
    if (!iDone) {
        iDone = true;
        emit success(aProfileName, aMessage);
    }
}
