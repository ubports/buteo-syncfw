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
#ifndef STORAGEBOOKER_H
#define STORAGEBOOKER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QMutex>

namespace Buteo {
    
/*! \brief A helper class for managing storage reservations.
 *
 */
class StorageBooker
{
public:

    //! \brief Constructor
    StorageBooker();

    //! \brief Destructor
    ~StorageBooker();

    /*! \brief Tries to reserve one storage for the given client.
     *
     * If the reserve is successfull, the caller must call release when
     * it does not need the storage anymore.
     * The same client can call reserve multiple times. Internal reference
     * counter is increased in that case. For each reserve there must be a
     * release call later. Other clients calling reserve for the same storage
     * will fail, while the storage is reserved to some other client.
     * \param aStorageName Name of the requested storage.
     * \param aClientId ID of the requesting client.
     * \return Success indicator.
     */
    bool reserveStorage(const QString &aStorageName,
                         const QString &aClientId = "");

    /*! \brief Tries to reserve multiple storages for the given client.
     *
     * If the reserve is successfull, the caller must call release for each
     * storage when it does not need the storages anymore.
     * The reserve is successfull only if all given storages are available.
     * If the reserve fails, no storages are reserved.
     * \param aStorageNames Names of the storages to reserve.
     * \param aClientId ID of the requesting client.
     * \return Success indicator.
     */
    bool reserveStorages(const QStringList &aStorageNames,
                         const QString &aClientId = "");

    /*! \brief Releases the given storage.
     *
     * \param aStorageName Name of the storage to release.
     * \return Number of remaining references to the storage. If this is zero,
     *  other clients can now reserve the storage.
     */
    unsigned releaseStorage(const QString &aStorageName);

    /*! \brief Releases the given storages.
     *
     * \param aStorageNames Names of the storages to release.
     */
    void releaseStorages(const QStringList &aStorageNames);

    /*! \brief Checks if the given storage is available for the given client.
     *
     * The storage is available if there are no reservations for it or if the
     * storage is already reserved for the same client. If the storage is
     * available, it can be reserved for the client by calling reserve.
     * \param aStorageName Name of the requested storage.
     * \param aClientId ID of the requesting client.
     * \return Is the storage available.
     */
    bool isStorageAvailable(const QString &aStorageName,
                            const QString &aClientId = "") const;

    /*! \brief Checks if the given storages are available for the given client.
     *
     * \param aStorageNames Names of the requested storages.
     * \param aClientId ID of the requesting client.
     * \return Are the storages available.
     */
    bool storagesAvailable(const QStringList &aStorageNames,
                           const QString &aClientId = "") const;

private:

    struct StorageMapItem
    {
        QString iClientId;
        unsigned iRefCount;

        StorageMapItem() : iRefCount(0) { };

        StorageMapItem(const QString &aClientId) :
            iClientId(aClientId), iRefCount(1) { };
    };

    QMap<QString, StorageMapItem> iStorageMap;

    mutable QMutex iMutex;

};

}

#endif // STORAGEBOOKER_H
