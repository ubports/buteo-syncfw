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

using namespace Buteo;

void ClientPluginTest::testCreateDestroy()
{
    QDir dir = QDir(QCoreApplication::applicationDirPath() + "/..");
    QString path = dir.absolutePath();
    if (dir.cd("../dummyplugins/dummyclient"))
    {
        path = dir.absolutePath();
    } // no else

    PluginManager pluginManager( path );

    SyncProfile profile( "dummyprofile" );
    ClientPlugin* client1 = pluginManager.createClient( "hdummy", profile, this );
    QVERIFY( client1 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    ClientPlugin* client2 = pluginManager.createClient( "hdummy", profile, this );

    QVERIFY( client2 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyClient( client1 );

    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyClient( client2 );

    QEXPECT_FAIL("", "According to a comment in PluginManager.cpp: KLUDGE: Due to NB #169065, crashes are seen in QMetaType if we unload DLLs. Hence commenting", Continue);
    QVERIFY( pluginManager.iLoadedDlls.count() == 0 );
}

QTEST_MAIN(Buteo::ClientPluginTest)
