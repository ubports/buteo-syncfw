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
#include "DummyClient.h"

using namespace Buteo;

extern "C" DummyClient* createPlugin( const QString& aPluginName,
                                      const SyncProfile& aProfile,
                                      PluginCbInterface *aCbInterface )
{
    return new DummyClient( aPluginName, aProfile, aCbInterface );
}

extern "C" void destroyPlugin( DummyClient* aClient )
{
    delete aClient;
}


DummyClient::DummyClient( const QString& aPluginName,
                          const SyncProfile& aProfile,
                          PluginCbInterface* aCbInterface )
 : ClientPlugin( aPluginName, aProfile, aCbInterface )
{

}

DummyClient::~DummyClient()
{

}

bool DummyClient::init()
{
    return true;
}

bool DummyClient::uninit()
{
    return true;
}

bool DummyClient::startSync()
{
    return true;
}

void DummyClient::abortSync()
{

}

bool DummyClient::cleanUp()
{
	return false;
}

SyncResults DummyClient::getSyncResults()
{
    SyncResults results;

    return results;
}

void DummyClient::connectivityStateChanged( Sync::ConnectivityType /*aType*/,
                                            bool /*aState*/ )
{

}
