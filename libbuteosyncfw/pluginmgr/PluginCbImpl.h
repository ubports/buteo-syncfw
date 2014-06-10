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
#ifndef PLUGINCBIMPL_H
#define PLUGINCBIMPL_H

#include "PluginCbInterface.h"
#include "PluginManager.h"
#include "SyncDaemonProxy.h"
#include "TransportTracker.h"

namespace Buteo {

class PluginCbImpl : public PluginCbInterface
{
public:
    PluginCbImpl();

    ~PluginCbImpl();

    /// \see PluginCbInterface::requestStorage
    virtual bool requestStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller);

    /// \see PluginCbInterface::releaseStorage
    virtual void releaseStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller);

    /// \see PluginCbInterface::createStorage
    virtual StoragePlugin* createStorage(const QString &aPluginName);

    /// \see PluginCbInterface::destroyStorage
    virtual void destroyStorage(StoragePlugin *aStorage);

    /// \see PluginCbInterface::isConnectivityAvailable
    virtual bool isConnectivityAvailable( Sync::ConnectivityType aType );

    /// \see PluginCbInterface::getSyncProfileByRemoteAddress
    virtual Profile* getSyncProfileByRemoteAddress(const QString& aAddress);

    /// \see PluginCbInterface::getValue
    virtual QString getValue(const QString& aAddress, const QString& aKey);

signals:
    
    //! emitted by releaseStorages call
    void storageReleased();

private:

    SyncDaemonProxy  *imsyncIface;

    PluginManager   iPluginManager;
    
    TransportTracker  iTransportTracker;
};

}

#endif // PLUGINCBIMPL_H
