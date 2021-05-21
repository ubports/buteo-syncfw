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

#include "ServerActivator.h"
#include "ProfileEngineDefs.h"
#include "LogMacros.h"

using namespace Buteo;

ServerActivator::ServerActivator(ProfileManager &aProfileManager,
                                 TransportTracker &aTransportTracker, QObject *aParent)
    :   QObject(aParent),
        iProfileManager(aProfileManager),
        iTransportTracker(aTransportTracker)
{
    FUNCTION_CALL_TRACE;

    // Get server profiles and transports used by them.
    QStringList serverProfileNames = iProfileManager.profileNames(
                                         Profile::TYPE_SERVER);
    foreach (QString serverProfileName, serverProfileNames) {
        Profile *serverProfile = iProfileManager.profile(serverProfileName,
                                                         Profile::TYPE_SERVER);
        if (serverProfile != 0) {
            if (serverProfile->isEnabled()) {
                ServerData data;
                data.iTransports = transportsFromProfile(serverProfile);

                // Normally server plug-in is loaded only when a transport
                // used by it is available. A server profile can force the
                // the plug-in to be always loaded be defining the following
                // key.
                if (serverProfile->boolKey(KEY_LOAD_WITHOUT_TRANSPORT)) {
                    data.iRefCount++;
                }

                iServers.insert(serverProfileName, data);
            }

            delete serverProfile;
            serverProfile = 0;
        } else {
            LOG_WARNING("Failed to load server profile:" << serverProfileName);
        }
    }

    // Add one reference per matching and enabled transport.
    QList<Sync::ConnectivityType> transports;
    transports.append(Sync::CONNECTIVITY_BT);
    transports.append(Sync::CONNECTIVITY_USB);
    transports.append(Sync::CONNECTIVITY_INTERNET);
    foreach (Sync::ConnectivityType transport, transports) {
        bool transportEnabled = iTransportTracker.isConnectivityAvailable(transport);
        foreach (QString serverName, iServers.keys()) {
            if (transportEnabled && iServers[serverName].iTransports.contains(transport)) {
                iServers[serverName].iRefCount++;
            }
        }
    }

    connect(&aTransportTracker, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)),
            this, SLOT(onConnectivityStateChanged(Sync::ConnectivityType, bool)));
}

ServerActivator::~ServerActivator()
{
    FUNCTION_CALL_TRACE;
}

int ServerActivator::addRef(const QString &aServerName, bool emitSignal /*= true*/)
{
    FUNCTION_CALL_TRACE;

    int refCount = 0;
    if (iServers.contains(aServerName)) {
        refCount = ++iServers[aServerName].iRefCount;
        if (emitSignal && (refCount == 1)) {
            emit serverEnabled(aServerName);
        }
    } else {
        LOG_WARNING("Unknown server:" << aServerName);
    }

    return refCount;
}

int ServerActivator::removeRef(const QString &aServerName, bool emitSignal /*= true*/)
{
    FUNCTION_CALL_TRACE;

    int refCount = 0;
    if (iServers.contains(aServerName)) {
        ServerData &data = iServers[aServerName];
        if (data.iRefCount > 0) {
            data.iRefCount--;
            if (emitSignal && data.iRefCount == 0) {
                emit serverDisabled(aServerName);
            }
        }
        refCount = data.iRefCount;
    } else {
        LOG_WARNING("Unknown server:" << aServerName);
    }

    return refCount;
}

QStringList ServerActivator::enabledServers() const
{
    FUNCTION_CALL_TRACE;

    QStringList enabledServers;
    foreach (QString serverName, iServers.keys()) {
        if (iServers[serverName].iRefCount > 0) {
            enabledServers.append(serverName);
        }
    }

    return enabledServers;
}

void ServerActivator::onConnectivityStateChanged(Sync::ConnectivityType aType,
                                                 bool aState)
{
    FUNCTION_CALL_TRACE;

    foreach (QString serverName, iServers.keys()) {
        if (iServers[serverName].iTransports.contains(aType)) {
            if (aState) {
                addRef(serverName);
            } else {
                removeRef(serverName);
            }
        }
    }
}

QList<Sync::ConnectivityType> ServerActivator::transportsFromProfile(
    const Profile *aProfile)
{
    FUNCTION_CALL_TRACE;

    QList<Sync::ConnectivityType> transports;
    if (aProfile != 0) {
        if (aProfile->boolKey(KEY_BT_TRANSPORT)) {
            transports.append(Sync::CONNECTIVITY_BT);
        }
        if (aProfile->boolKey(KEY_USB_TRANSPORT)) {
            transports.append(Sync::CONNECTIVITY_USB);
        }
        if (aProfile->boolKey(KEY_INTERNET_TRANSPORT)) {
            transports.append(Sync::CONNECTIVITY_INTERNET);
        }
    }

    return transports;
}
