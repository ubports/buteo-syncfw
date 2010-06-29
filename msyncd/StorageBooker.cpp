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


#include "StorageBooker.h"
#include <QMutexLocker>
#include "LogMacros.h"

using namespace Buteo;

StorageBooker::StorageBooker()
:   iMutex(QMutex::Recursive)
{
    FUNCTION_CALL_TRACE;
}

StorageBooker::~StorageBooker()
{
    FUNCTION_CALL_TRACE;
}

bool StorageBooker::reserveStorage(const QString &aStorageName,
                                   const QString &aClientId)
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    bool success = false;

    if (iStorageMap.contains(aStorageName))
    {
        StorageMapItem item = iStorageMap[aStorageName];
        if (aClientId.isEmpty() || aClientId != item.iClientId)
        {
            // Already reserved for different client.
            success = false;
        }
        else
        {
            // Already reserved for the same client. Increase ref count.
            item.iRefCount++;
            iStorageMap[aStorageName] = item;
            success = true;
        }
    }
    else
    {
        // No reservations for the storage. Add a new entry to the storage
        // reservation map.
        iStorageMap.insert(aStorageName, aClientId);
        success = true;
    }

    return success;
}

bool StorageBooker::reserveStorages(const QStringList &aStorageNames,
                                    const QString &aClientId)
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    bool success = false;
    if (storagesAvailable(aStorageNames, aClientId))
    {
        foreach (QString storage, aStorageNames)
        {
            reserveStorage(storage, aClientId);
        }
        success = true;
    }
    else
    {
        success = false;
    }

    return success;
}

unsigned StorageBooker::releaseStorage(const QString &aStorageName)
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    unsigned remainingRefCount = 0;

    if (iStorageMap.contains(aStorageName))
    {
        StorageMapItem item = iStorageMap[aStorageName];
        item.iRefCount--;
        remainingRefCount = item.iRefCount;

        if (remainingRefCount == 0)
        {
            iStorageMap.remove(aStorageName);
        }
        else
        {
            iStorageMap[aStorageName] = item;
        }
    } // no else

    return remainingRefCount;
}

void StorageBooker::releaseStorages(const QStringList &aStorageNames)
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    foreach (QString storage, aStorageNames)
    {
        releaseStorage(storage);
    }
}

bool StorageBooker::isStorageAvailable(const QString &aStorageName,
                                       const QString &aClientId) const
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    return (!iStorageMap.contains(aStorageName) ||
            (!aClientId.isEmpty() &&
             (aClientId == iStorageMap[aStorageName].iClientId)));

}

bool StorageBooker::storagesAvailable(const QStringList &aStorageNames,
                                      const QString &aClientId) const
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    foreach (QString storage, aStorageNames)
    {
        if (!isStorageAvailable(storage, aClientId))
            return false;
    }

    return true;
}
