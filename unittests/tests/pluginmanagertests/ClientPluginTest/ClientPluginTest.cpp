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

#include "ClientPluginTest.h"


#include "PluginManager.h"
#include "SyncProfile.h"

#define TEST_PLUGIN_PATH "/opt/tests/buteo-syncfw"

using namespace Buteo;

void ClientPluginTest::testCreateDestroy()
{
    PluginManager pluginManager( TEST_PLUGIN_PATH );

    SyncProfile profile( "dummyprofile" );
    ClientPlugin *client1 = pluginManager.createClient( "hdummy", profile, this );
    QVERIFY( client1 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    ClientPlugin *client2 = pluginManager.createClient( "hdummy", profile, this );

    QVERIFY( client2 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyClient( client1 );

    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyClient( client2 );

    QVERIFY( pluginManager.iLoadedDlls.count() == 0 );
}

QTEST_GUILESS_MAIN(Buteo::ClientPluginTest)
