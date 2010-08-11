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
#include "DummyStorage.h"

using namespace Buteo;

extern "C" StoragePlugin* createPlugin( const QString& aPluginName )
{
    return new DummyStorage( aPluginName );
}

extern "C" void destroyPlugin( StoragePlugin* aStorage )
{
    delete aStorage;
}


DummyStorage::DummyStorage( const QString& aPluginName )
 : StoragePlugin( aPluginName )
{

}

DummyStorage::~DummyStorage()
{

}

bool DummyStorage::init( const QMap<QString, QString>& /*aProperties*/ )
{
    return true;
}

bool DummyStorage::uninit()
{
    return true;
}

bool DummyStorage::getAllItems( QList<StorageItem*>& /*aItems*/ )
{
    return true;
}

bool DummyStorage::getNewItems( QList<StorageItem*>& /*aNewItems*/, const QDateTime& /*aTime*/ )
{
    return true;
}

bool DummyStorage::getModifiedItems( QList<StorageItem*>& /*aModifiedItems*/, const QDateTime& /*aTime*/ )
{
    return true;
}

bool DummyStorage::getDeletedItems( QList<StorageItem*>& /*aDeletedItems*/, const QDateTime& /*aTime*/ )
{
    return true;
}

bool DummyStorage::getAllItemIds( QList<QString>& /*aItems*/ )
{
    return true;
}

bool DummyStorage::getNewItemIds( QList<QString>& /*aNewItems*/, const QDateTime& /*aTime*/ )
{
    return true;
}

bool DummyStorage::getModifiedItemIds( QList<QString>& /*aModifiedItems*/, const QDateTime& /*aTime*/ )
{
    return true;
}

bool DummyStorage::getDeletedItemIds( QList<QString>& /*aDeletedItems*/, const QDateTime& /*aTime*/ )
{
    return true;
}

StorageItem* DummyStorage::newItem()
{
    return NULL;
}

StorageItem* DummyStorage::getItem( const QString& /*aItemId*/ )
{
    return NULL;
}

StoragePlugin::OperationStatus DummyStorage::addItem( StorageItem& /*aItem*/ )
{
    return STATUS_OK;
}

QList<StoragePlugin::OperationStatus> DummyStorage::addItems( const QList<StorageItem*>& aItems )
{
    QList<OperationStatus> statuses;

    for( int i = 0; i < aItems.count(); ++i ) {
        statuses.append( STATUS_OK );
    }

    return statuses;
}

StoragePlugin::OperationStatus DummyStorage::modifyItem( StorageItem& /*aItem*/ )
{
    return STATUS_OK;
}

QList<StoragePlugin::OperationStatus> DummyStorage::modifyItems( const QList<StorageItem*>& aItems )
{
    QList<OperationStatus> statuses;

    for( int i = 0; i < aItems.count(); ++i ) {
        statuses.append( STATUS_OK );
    }

    return statuses;
}

StoragePlugin::OperationStatus DummyStorage::deleteItem( const QString& /*aItemId*/ )
{
    return STATUS_OK;
}

QList<StoragePlugin::OperationStatus> DummyStorage::deleteItems( const QList<QString>& aItemIds )
{
    QList<OperationStatus> statuses;

    for( int i = 0; i < aItemIds.count(); ++i ) {
        statuses.append( STATUS_OK );
    }

    return statuses;
}
