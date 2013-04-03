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
#ifndef STORAGEPLUGIN_H_89943984899843
#define STORAGEPLUGIN_H_89943984899843

#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>

namespace Buteo {

class StorageItem;

/*! \brief Base class for storage plugins
 *
 */
class StoragePlugin
{

public:

    /*! \brief Status of operation performed by storage plugin
     *
     */
    enum OperationStatus
    {
        STATUS_INVALID_FORMAT = -6,  /*!< Operation failed because data is in invalid format*/
        STATUS_STORAGE_FULL = -5,    /*!< Operation failed because storage is full*/
        STATUS_OBJECT_TOO_BIG = -4,  /*!< Operation failed because object is too big*/
        STATUS_ERROR = -3,           /*!< General error occurred during operation*/
        STATUS_DUPLICATE = -2,       /*!< Operation was not performed as object was duplicate*/
        STATUS_NOT_FOUND = -1,       /*!< Operation failed as object was not found*/
        STATUS_OK = 0                /*!< Operation was completed successfully*/
    };

    /*! \brief Constructor
     *
     * @param aPluginName Name of this storage plugin
     */
    StoragePlugin( const QString& aPluginName );

    /*! \brief Destructor
     *
     */
    virtual ~StoragePlugin();

    /*! \brief Returns the name of this plugin
     *
     * @return Name of the plugin
     */
    const QString& getPluginName() const;

    /*! \brief Returns the value of the given property
     *
     * @param aProperty Property
     * @return Property value if property found, otherwise empty string
     */
    QString getProperty( const QString& aProperty ) const;

    /*! \brief Returns the properties set for this plugin
     *
     * @param aProperties Properties that are set
     */
    void getProperties( QMap<QString, QString>& aProperties ) const;

    /*! \brief Initializes the plugin
    *
    * It is recommended that the plugin should do not do any thread insecure
    * initializations inside constructor, instead it should do inside this method.
    * Parameters that were read from storage profile are passed as parameter to this
    * function.
    *
    * @param aProperties Properties that should be set for this plugin
    */
    virtual bool init( const QMap<QString, QString>& aProperties ) = 0;

    /*! \brief Uninitializes the plugin
     *
     */
    virtual bool uninit() = 0;

    /*! \brief Returns all known items
     *
     * @param aItems Array where to place items
     * @return True on success, otherwise false
     */
    virtual bool getAllItems( QList<StorageItem*>& aItems ) = 0;

    /*! \brief Returns id's of all known items
     *
     * @param aItems Array where to place item id's
     * @return True on success, otherwise false
     */
    virtual bool getAllItemIds( QList<QString>& aItems ) = 0;

    /*! \brief Returns all new items since aTime
     *
     * @param aNewItems Array where to place items
     * @param aTime Items with creation time > aTime are returned
     * @return True on success, otherwise false
     */
    virtual bool getNewItems( QList<StorageItem*>& aNewItems, const QDateTime& aTime ) = 0;

    /*! \brief Returns id's of all new items since aTime (creation time > aTime)
     *
     * @param aNewItemIds Array where to place item id's
     * @param aTime Items with creation time > aTime are returned
     * @return True on success, otherwise false
     */
    virtual bool getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime ) = 0;

    /*! \brief Returns all modified items since aTime
     *
     * @param aModifiedItems Array where to place items
     * @param aTime Items with modification time > aTime and creation time <=
     *  aTime are returned.
     * @return True on success, otherwise false
     */
    virtual bool getModifiedItems( QList<StorageItem*>& aModifiedItems, const QDateTime& aTime ) = 0;

    /*! \brief Returns id's of all modified items since aTime
     *
     * @param aModifiedItemIds Array where to place item id's
     * @param aTime Items with modification time > aTime and creation time <=
     *  aTime are returned.
     * @return True on success, otherwise false
     */
    virtual bool getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime ) = 0;

    /*! \brief Returns id's of all deleted items since aTime
     *
     * @param aDeletedItemIds Array where to place item id's
     * @param aTime Items with deletion time > aTime and creation time <=
     *  aTime are returned.
     * @return True on success, otherwise false
     */
    virtual bool getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime ) = 0;

    /*! \brief Generates a new item
     *
     * Returned item is temporary. Therefore returned item ALWAYS has its id
     * set as empty ID (""). ID will be assigned only after addItem() has been
     * called for the item.
     *
     * @return On success pointer to the item generated, otherwise NULL
     */
    virtual StorageItem* newItem() = 0;

    /*! \brief Returns an item based on id
     *
     * @param aItemId Id of the item to return
     * @return On success pointer to the item, otherwise NULL
     */
    virtual StorageItem* getItem( const QString& aItemId ) = 0;

    /*! \brief Returns items based on ids
     *
     * @param aItemIdList List of id's
     * @return List of items
     */
    virtual QList<StorageItem*> getItems( const QStringList& aItemIdList ) = 0;

    /*! \brief Adds an item to the storage
     *
     * Upon successful addition, item is updated with its
     * assigned ID.
     *
     * @param aItem Item to add
     * @return Operation status code
     */
    virtual OperationStatus addItem( StorageItem& aItem ) = 0;

    /*! \brief Adds items to the storage
     *
     * Upon successful addition, items are updated with its
     * assigned ID.
     *
     * @param aItems Items to add
     * @return Operation status codes
     */
    virtual QList<OperationStatus> addItems( const QList<StorageItem*>& aItems ) = 0;

    /*! \brief Modifies an item in the storage
     *
     * @param aItem Item to modify
     * @return Operation status code
     */
    virtual OperationStatus modifyItem( StorageItem& aItem ) = 0;

    /*! \brief Modifies item in the storage
     *
     * @param aItems Items to add
     * @return Operation status codes
     */
    virtual QList<OperationStatus> modifyItems( const QList<StorageItem*>& aItems ) = 0;

    /*! \brief Deletes an item from the storage
     *
     * @param aItemId Id of the item to be deleted
     * @return Operation status code
     */
    virtual OperationStatus deleteItem( const QString& aItemId ) = 0;

    /*! \brief Deletes an item from the storage
     *
     * @param aItemIds Id's of the item to be deleted
     * @return Operation status codes
     */
    virtual QList<OperationStatus> deleteItems( const QList<QString>& aItemIds ) = 0;

protected:

    //! Name of the plugin
    QString                   iPluginName;

    //! Properties of the plugin as read from profile xml
    QMap<QString, QString>    iProperties;
};

}

#endif // STORAGEPLUGIN_H_89943984899843
