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
#ifndef DUMMYCLIENT_H
#define DUMMYCLIENT_H

#include "ClientPlugin.h"
#include "SyncPluginLoader.h"

namespace Buteo {

class DummyClient : public ClientPlugin
{
    Q_OBJECT
public:

    DummyClient( const QString& aPluginName,
                 const SyncProfile& aProfile,
                 PluginCbInterface* aCbInterface );

    ~DummyClient();

    virtual bool init();

    virtual bool uninit();

    virtual bool startSync();

    virtual void abortSync();

    virtual bool cleanUp();

    virtual SyncResults getSyncResults();

public slots:

    virtual void connectivityStateChanged( Sync::ConnectivityType aType,
                                           bool aState );

};


class DummyClientLoader : public Buteo::SyncPluginLoader
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.buteo.msyncd.test.DummyClientLoader")
    Q_INTERFACES(Buteo::SyncPluginLoader)

public:
    ClientPlugin* createClientPlugin( const QString& aPluginName,
                                      const Buteo::SyncProfile& aProfile,
                                      Buteo::PluginCbInterface* aCbInterface ) override;
};

}

#endif  //  DUMMYCLIENT_H
