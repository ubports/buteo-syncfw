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
#ifndef DELETEDITEMSIDSTORAGE_H
#define DELETEDITEMSIDSTORAGE_H

#include <QSqlDatabase>
#include <QDateTime>

namespace Buteo {
    
/*!
 * \brief Persistent storage for storing deleted item IDs
 */
class DeletedItemsIdStorage{
public:
    
    /**
     * \brief Contructor
     */
    DeletedItemsIdStorage();

    /**
     * \brief Destructor
     */
    ~DeletedItemsIdStorage();

    /*! \brief Initializes backend
     *
     *
     * @param aDbFile Path to database to use as persistent storage
     * @return True on success, otherwise false
     */
    bool init( const QString& aDbFile );

    /*! \brief Uninitializes backend
     *
     * @return True on success, otherwise false
     */
    bool uninit();

    /*! \brief Retrieves persistently stored snapshot of item id's
     *
     * @param aItems Items of the snapshot
     * @param aCreationTimes Creation times of the items
     * @return True on success, otherwise false
     */
    bool getSnapshot( QList<QString>& aItems, QList<QDateTime>& aCreationTimes ) const;

    /*! \brief Store a snapshot of item id's persistently
     *
     * @param aItems Item id's to store
     * @param aCreationTimes Creation times of the items
     * @return True on success, otherwise false
     */
    bool setSnapshot( const QList<QString>& aItems, const QList<QDateTime>& aCreationTimes );

    /*! \brief Adds a deleted item to backend
     *
     * @param aItem Item Id
     * @param aCreationTime Time when item was initially created
     * @param aDeleteTime Time of deletion
     */
    void addDeletedItem( const QString& aItem, const QDateTime& aCreationTime, const QDateTime& aDeleteTime );

    /*! \brief Adds deleted items to backend
     *
     * @param aItems Items Ids
     * @param aCreationTimes Times when the items were initially created
     * @param aDeleteTimes Times of deletion
     */
    void addDeletedItems( const QList<QString>& aItems, const QList<QDateTime>& aCreationTimes,
                          const QList<QDateTime>& aDeleteTimes );

    /*! \brief Returns the deleted items after given time
     *
     * @param aItems Returned deleted items
     * @param aTime Items deleted after this time are considered deleted
     * @return True on success, otherwise false
     */
    bool getDeletedItems( QList<QString>& aItems, const QDateTime& aTime );

protected:

    /**
     * \brief Checks whether snapshot table exists and creates it if needed
     * @return True on success, otherwise false
     */
    bool ensureItemSnapshotExists();

    /**
     * \brief Checks whether item id table exists and creates it if needed
     * @return True on success, otherwise false
     */
    bool ensureDeletedItemsExists();

private:

    QSqlDatabase    iDb;                ///< Database handle
    QString         iConnectionName;    ///< Database connection ID string

};

}

#endif
