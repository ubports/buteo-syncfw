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
#ifndef SERVERPLUGINTEST_H
#define SERVERPLUGINTEST_H

#include <QtTest/QtTest>

#include "PluginCbInterface.h"

namespace Buteo {
    
class ServerPluginTest : public QObject, public PluginCbInterface
{
Q_OBJECT;

public:
    virtual bool requestStorage(const QString &/*aStorageName*/,
                                const SyncPluginBase */*aCaller*/)
    {
        return false;
    }

    virtual void releaseStorage(const QString &/*aStorageName*/,
                                const SyncPluginBase */*aCaller*/) { }


    virtual StoragePlugin* createStorage(const QString &/*aPluginName*/)
    {
        return NULL;
    }

    virtual void destroyStorage(StoragePlugin */*aStorage*/) { }

    virtual QString getDeviceIMEI() { return QString( "000000000000000" ); }

    virtual bool isConnectivityAvailable( Sync::ConnectivityType /*aType*/ ) { return false; }

    virtual Profile* getSyncProfileByRemoteAddress(const QString& aAddress)
    {
        Q_UNUSED(aAddress);
        return 0;
    }

    virtual QString getValue(const QString& aAddress, const QString& aKey)
    {
        Q_UNUSED(aAddress);
        Q_UNUSED(aKey);
        return "";
    }

private slots:

    void testCreateDestroy();

private:

};

}

#endif
