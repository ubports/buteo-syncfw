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
#include "DummyServer.h"

using namespace Buteo;

DummyServer::DummyServer( const QString& aPluginName,
                          const Profile& aProfile,
                          PluginCbInterface *aCbInterface )
 : ServerPlugin( aPluginName, aProfile, aCbInterface )
{

}

DummyServer::~DummyServer()
{

}

bool DummyServer::init()
{
    return true;
}

bool DummyServer::uninit()
{
    return true;
}

bool DummyServer::startListen()
{
    return true;
}

bool DummyServer::cleanUp()
{
    return true;
}
void DummyServer::stopListen()
{

}

void DummyServer::suspend()
{
}

void DummyServer::resume()
{
}

void DummyServer::connectivityStateChanged( Sync::ConnectivityType /*aType*/,
                                            bool /*aState*/ )
{

}


ServerPlugin* DummyServerLoader::createServerPlugin( const QString& aPluginName,
                                                     const Buteo::Profile& aProfile,
                                                     Buteo::PluginCbInterface* aCbInterface )
{
    return new DummyServer( aPluginName, aProfile, aCbInterface );
}

