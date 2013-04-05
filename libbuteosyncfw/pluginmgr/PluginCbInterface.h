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
#ifndef PLUGINCBINTERFACE_H
#define PLUGINCBINTERFACE_H

#include <QString>

#include "SyncPluginBase.h"

namespace Buteo {

class StoragePlugin;
class Profile;
             
/*! \brief Interface which client and server plugins can use to communicate with
 *         synchronization daemon
 */
class PluginCbInterface
{
public:

    virtual ~PluginCbInterface() {}

    /*! \brief Tries to reserve the given storage to the caller.
     *
     * Server plug-ins must reserve storages before using them. For client
     * plug-ins this is done automatically by the sync daemon when sync starts,
     * based on profiles.
     * Release must be called when the successfully requested storage is not
     * needed anymore.
     * @param aStorageName Name of the storage backend to reserve.
     * @param aCaller Object calling this function
     * @return Success indicator
     */
    virtual bool requestStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller) = 0;

    /*! \brief Releases the given storage.
     *
     * Call this function after a storage requested with requestStorage is not
     * needed anymore by the caller.
     * \param aStorageName Name of the storage backend to release.
     * \param aCaller Object calling this function.
     */
    virtual void releaseStorage(const QString &aStorageName,
                                const SyncPluginBase *aCaller) = 0;

    /*! \brief Creates a storage plug-in instance.
     *
     * Server plug-ins must reserve the storage backend before creating a
     * plug-in that uses it. Otherwise simultaenous access of the same backend
     * may lead to inconsistent state.
     * \param aPluginName Name of the storage plug-in to create.
     * \return Created plug-in instance. NULL if failed.
     */
    virtual StoragePlugin* createStorage(const QString &aPluginName) = 0;

    /*! \brief Destroys the given storage plug-in instance.
     *
     * \param aStorage Storage plug-in to destroy.
     */
    virtual void destroyStorage(StoragePlugin *aStorage) = 0;

    /*! \brief Returns whether connectivity domain is available
     *
     * @param aType Type of connectivity domain
     * @return True if connectivity domain is available, otherwise false
     */
    virtual bool isConnectivityAvailable( Sync::ConnectivityType aType ) = 0;

    /*! \brief tries to fetch a profile object based on the remote party's
     * address (BT address for eg)
     *
     * @param aAddress remote party's address
     * @return profile object - to be deleted by caller
     */
    virtual Profile* getSyncProfileByRemoteAddress(const QString& aAddress) = 0;

    /*! \brief Get a value for a property describing the remote device
     *
     * @param aAddress remote device's address
     * @param aKey profile key identifying the property
     * @return value for the property
     */
    virtual QString getValue(const QString& aAddress, const QString& aKey) = 0;
};

}

#endif
