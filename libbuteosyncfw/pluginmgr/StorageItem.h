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
#ifndef STORAGEITEM_H
#define STORAGEITEM_H

#include <QtGlobal>
#include <QString>
#include <QByteArray>


namespace Buteo {
    
/*! \brief Class to describe a storable item
 *
 */
class StorageItem
{
public:

    /*! \brief Constructor
     *
     */
    StorageItem();

    /*! \brief Destructor
     *
     */
    virtual ~StorageItem();

    /*! \brief Sets the id of the item
     *
     * @param aId The id of the item
     */
    void setId( const QString &aId );

    /*! \brief Returns the id of the item
     *
     * @return Id of the item
     */
    const QString& getId() const;

    /*! \brief Sets the id of the parent of this item
     *
     * @param aParentId The id of the parent of item
     */
    void setParentId( const QString& aParentId );

    /*! \brief Returns the id of the parent of this item
     *
     * @return Id of the parent of this item
     */
    const QString& getParentId() const;

    /*! \brief Sets the type of this item
     *
     * @param aType Type of this item
     */
    void setType( const QString& aType );

    /*! \brief Gets the type of this item
     *
     * @return Type of this item
     */
    const QString& getType() const;

    /*! \brief Sets the version of this item
     *
     * @param aVersion Version of this item
     */
    void setVersion( const QString& aVersion );

    /*! \brief Gets the version of this item
     *
     * @return Version of this item
     */
    const QString& getVersion() const;

    /*! \brief Write (part of) the item data
     *
     * @param aOffset Offset to start writing from
     * @param aData Data buffer to write. All bytes from buffer are written
     * @return True on success, otherwise false
     */
    virtual bool write( qint64 aOffset, const QByteArray& aData ) = 0;

    /*! \brief Read (part of) the item data
     *
     * @param aOffset The offset in bytes from where the data is read
     * @param aLength The number of bytes to read
     * @param aData Data buffer where to place data
     * @return True on success, otherwise false
     */
    virtual bool read( qint64 aOffset, qint64 aLength, QByteArray& aData ) const = 0;

    /*! \brief Sets the length of the item data
    *
    * @param aLen Length to set for item data
    * @return True on success, otherwise false
    */
    virtual bool resize( qint64 aLen ) = 0;

    /*! \brief Get the size of the item data
     *
     * @return The data size in bytes
     */
    virtual qint64 getSize() const = 0;

private:

    QString iId;
    QString iParentId;
    QString iType;
    QString iVersion;

};

}

#endif
