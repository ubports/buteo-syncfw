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
#ifndef DUMMYSERVER_H
#define DUMMYSERVER_H

#include "ServerPlugin.h"
#include "SyncPluginLoader.h"

namespace Buteo {
    
class DummyServer : public ServerPlugin
{
    Q_OBJECT

public:

    DummyServer( const QString& aPluginName,
                 const Profile& aProfile,
                 PluginCbInterface *aCbInterface );

    virtual ~DummyServer();

    virtual bool init();

    virtual bool uninit();

    virtual bool startListen();

    virtual void stopListen();

    virtual void suspend();

    virtual void resume();

    virtual bool cleanUp();

public slots:

    virtual void connectivityStateChanged( Sync::ConnectivityType aType,
                                           bool aState );
};

class DummyServerLoader : public Buteo::SyncPluginLoader
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.buteo.msyncd.test.DummyServerLoader")
    Q_INTERFACES(Buteo::SyncPluginLoader)

public:
    ServerPlugin* createServerPlugin( const QString& aPluginName,
                                      const Buteo::Profile& aProfile,
                                      Buteo::PluginCbInterface* aCbInterface ) override;
};

}

#endif  //  DUMMYSERVER_H
