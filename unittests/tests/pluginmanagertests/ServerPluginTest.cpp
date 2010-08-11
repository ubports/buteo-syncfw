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
#include "ServerPluginTest.h"

#include "SyncFwTestLoader.h"

#include "PluginManager.h"
#include "SyncProfile.h"

using namespace Buteo;

void ServerPluginTest::testCreateDestroy()
{
    QDir dir = QDir::current();
    QString path = dir.absolutePath();
    if (dir.cd("../dummyplugins/dummyserver"))
    {
        path = dir.absolutePath();
    } // no else

    PluginManager pluginManager( path );

    SyncProfile profile( "dummyprofile" );
    ServerPlugin* server1 = pluginManager.createServer( "hdummy", profile, this );
    QVERIFY( server1 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    ServerPlugin* server2 = pluginManager.createServer( "hdummy", profile, this );

    QVERIFY( server2 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyServer( server1 );

    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyServer( server2 );

    QVERIFY( pluginManager.iLoadedDlls.count() == 0 );
}

TESTLOADER_ADD_TEST(ServerPluginTest);
