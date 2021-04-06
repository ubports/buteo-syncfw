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

#include "PluginRunnerTest.h"

using namespace Buteo;

const QString PLUGIN = "plugin";
const QString PROFILE = "profile";

void PluginRunnerTest::initTestCase()
{
    iSProfile = new SyncProfile(PROFILE);
    iPManager = new PluginManager();
    iClientPRunner = new ClientPluginRunner(PLUGIN, iSProfile, iPManager, NULL, NULL);
    iPRunner = iClientPRunner;
}
void PluginRunnerTest::testPluginRunnerConstructor()
{
    QCOMPARE(iPRunner->iInitialized, false);
    QCOMPARE(iPRunner->iPluginMgr, iPManager);
    QVERIFY(iPRunner->iPluginCbIf == NULL);
    QCOMPARE(iPRunner->iType, PluginRunner::PLUGIN_CLIENT);
    QCOMPARE(iPRunner->pluginType(), PluginRunner::PLUGIN_CLIENT);
    QCOMPARE(iPRunner->pluginName(), PLUGIN);
}
void PluginRunnerTest::cleanupTestCase()
{
    QVERIFY(iPManager != 0);
    delete iPManager;
    QVERIFY(iSProfile != 0);
    delete iSProfile;
    QVERIFY(iClientPRunner != 0);
    delete iClientPRunner;

    iPManager = 0;
    iSProfile = 0;
    iClientPRunner = 0;
    iPRunner = 0;
}



QTEST_MAIN(Buteo::PluginRunnerTest)
