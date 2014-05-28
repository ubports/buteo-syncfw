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
#include "PluginCbImpl.h"
#include "SyncPluginBase.h"
#include "StoragePlugin.h"
#include "SyncCommonDefs.h"
#include "Profile.h"
#include "LogMacros.h"

using namespace Buteo;

PluginCbImpl::PluginCbImpl()
{
    FUNCTION_CALL_TRACE;

    imsyncIface = new SyncDaemonProxy( "com.meego.msyncd",
                                       "/synchronizer",
                                       QDBusConnection::sessionBus());
}

PluginCbImpl::~PluginCbImpl()
{
    FUNCTION_CALL_TRACE;

    if( imsyncIface ) {
        delete imsyncIface;
        imsyncIface = 0;
    }
}

bool PluginCbImpl::requestStorage(const QString &aStorageName,
        const SyncPluginBase */*aCaller*/)
{
    FUNCTION_CALL_TRACE;

    bool requestResult = false;

    if( imsyncIface ) {
        QStringList storages;
    	storages << aStorageName;
    	QDBusReply<bool> gotStorages = imsyncIface->requestStorages(storages);

    	if( !gotStorages.isValid() )
        	LOG_WARNING( "Request for storage " << aStorageName << " failed" );
    	else
            requestResult = gotStorages.value();
    } else {
        LOG_WARNING( "msyncd dbus interface is NULL" );
    }

    return requestResult;
}

void PluginCbImpl::releaseStorage(const QString &aStorageName,
        const SyncPluginBase */*aCaller*/)
{
    FUNCTION_CALL_TRACE;

    if( imsyncIface ) {
        QStringList storages;
    	storages << aStorageName;
    	imsyncIface->releaseStorages( storages );
    } else {
        LOG_WARNING( "msyncd dbus interface is NULL" );
    }
}

StoragePlugin* PluginCbImpl::createStorage(const QString &aPluginName)
{
    FUNCTION_CALL_TRACE;

    StoragePlugin* plugin = NULL;
    if ( !aPluginName.isEmpty() ) {
        plugin = iPluginManager.createStorage(aPluginName);
    } // no else

    return plugin;
}

void PluginCbImpl::destroyStorage(StoragePlugin *aStorage)
{
    FUNCTION_CALL_TRACE;

    iPluginManager.destroyStorage(aStorage);
}

bool PluginCbImpl::isConnectivityAvailable( Sync::ConnectivityType aType )
{
    FUNCTION_CALL_TRACE;

    return iTransportTracker.isConnectivityAvailable(aType);
}

Profile* PluginCbImpl::getSyncProfileByRemoteAddress(const QString& aAddress)
{
    Q_UNUSED( aAddress );
    return NULL;
}

QString PluginCbImpl::getValue(const QString& aAddress, const QString& aKey)
{
    Q_UNUSED( aAddress );
    Q_UNUSED( aKey );

    return "";
}
