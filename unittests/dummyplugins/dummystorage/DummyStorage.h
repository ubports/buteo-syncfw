/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2013 - 2021 Jolla Ltd.
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
#ifndef DUMMYSTORAGE_H
#define DUMMYSTORAGE_H

#include "StoragePlugin.h"
#include "StoragePluginLoader.h"

namespace Buteo {

class DummyStorage : public StoragePlugin
{
public:

    DummyStorage( const QString &aPluginName );

    virtual ~DummyStorage();

    virtual bool init( const QMap<QString, QString> &aProperties );

    virtual bool uninit();

    virtual bool getAllItems( QList<StorageItem *> &aItems );

    virtual bool getNewItems( QList<StorageItem *> &aNewItems, const QDateTime &aTime );

    virtual bool getModifiedItems( QList<StorageItem *> &aModifiedItems, const QDateTime &aTime );

    virtual bool getDeletedItems( QList<StorageItem *> &aDeletedItems, const QDateTime &aTime );

    virtual bool getAllItemIds( QList<QString> &aItems );

    virtual bool getNewItemIds( QList<QString> &aNewItems, const QDateTime &aTime );

    virtual bool getModifiedItemIds( QList<QString> &aModifiedItems, const QDateTime &aTime );

    virtual bool getDeletedItemIds( QList<QString> &aDeletedItems, const QDateTime &aTime );

    virtual StorageItem *newItem();

    virtual StorageItem *getItem( const QString &aItemId );

    virtual QList<StorageItem *> getItems(const QStringList &aItemIdList );

    virtual OperationStatus addItem( StorageItem &aItem );

    virtual QList<OperationStatus> addItems( const QList<StorageItem *> &aItems );

    virtual OperationStatus modifyItem( StorageItem &aItem );

    virtual QList<OperationStatus> modifyItems( const QList<StorageItem *> &aItems );

    virtual OperationStatus deleteItem( const QString &aItemId );

    virtual QList<OperationStatus> deleteItems( const QList<QString> &aItemIds );

};


class DummyStorageLoader : public Buteo::StoragePluginLoader
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.buteo.msyncd.test.DummyStorageLoader")
    Q_INTERFACES(Buteo::StoragePluginLoader)

public:
    StoragePlugin *createPlugin( const QString &aPluginName );
};

}

#endif  //  DUMMYSTORAGE_H
