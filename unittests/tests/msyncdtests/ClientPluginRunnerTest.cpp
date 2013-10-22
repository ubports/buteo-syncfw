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
#include "SyncFwTestLoader.h"
#include "ClientPluginRunnerTest.h"
#include "PluginManager.h"
#include "synchronizer.h"
#include "ClientThread.h"

using namespace Buteo;

const QString PROFILE = "Profile";
const QString PLUGIN = "plugin";

void ClientPluginRunnerTest::initTestCase()
{
	iSprofile = new SyncProfile(PROFILE);
	iPluginMgr = new PluginManager();
	iPluginCbIf =  new Synchronizer(NULL);
	iClientPrunner = new ClientPluginRunner(PLUGIN,iSprofile, iPluginMgr,
						iPluginCbIf, NULL);
}
void ClientPluginRunnerTest::cleanupTestCase()
{
	QVERIFY(iSprofile);
	delete iSprofile;
	QVERIFY(iClientPrunner);
	delete iClientPrunner;
	QVERIFY(iPluginMgr);
	delete iPluginMgr;
    QVERIFY(iPluginCbIf);
    delete iPluginCbIf;

	iSprofile = 0;
	iPluginMgr = 0;
	iPluginCbIf = 0;
	iClientPrunner = 0;
}
void ClientPluginRunnerTest::testCpluginRunnerConstructor()
{
	SyncResults syncRes;
	bool dateTime;
	QDateTime current;

	QVERIFY(iClientPrunner->plugin() == 0);
	QVERIFY(iClientPrunner->iThread == 0);
	QCOMPARE(iClientPrunner->iProfile, iSprofile);
	QCOMPARE(iClientPrunner->pluginType(), PluginRunner::PLUGIN_CLIENT);
	QCOMPARE(iClientPrunner->pluginName(), PLUGIN);
	QCOMPARE(iClientPrunner->iInitialized, false);
	QVERIFY(iClientPrunner->iPluginMgr != 0);
	QVERIFY(iClientPrunner->iPluginCbIf != 0);

	//test syncResults()
	current = QDateTime::currentDateTime();
	syncRes = iClientPrunner->syncResults();
    QCOMPARE(syncRes.majorCode(), 0);
	QCOMPARE(syncRes.isScheduled(), false);
	dateTime = syncRes.syncTime() >= current;
	QVERIFY(dateTime);
}
void ClientPluginRunnerTest::testInit()
{
	SyncResults syncRes;

	iClientPrunner->iInitialized = true;
	QCOMPARE(iClientPrunner->init(), true);
	iClientPrunner->iInitialized = false;

	QVERIFY(iClientPrunner->iPluginMgr != 0);
	QVERIFY(iClientPrunner->iPluginCbIf != 0);
	QVERIFY(iClientPrunner->iProfile != 0);
	QCOMPARE(iClientPrunner->init(), false);
	QVERIFY(iClientPrunner->plugin() == 0);
	QVERIFY(iClientPrunner->iThread == 0);
	iClientPrunner->iInitialized = true;
	QCOMPARE(iClientPrunner->iInitialized, true);
}
void ClientPluginRunnerTest::testStart()
{
	//test start()
	QCOMPARE(iClientPrunner->iInitialized, true);	
	QVERIFY(iClientPrunner->iThread == 0);
	QCOMPARE(iClientPrunner->start(), false);

	iClientPrunner->iThread = new ClientThread();
	QVERIFY(iClientPrunner->iThread != 0);
	QCOMPARE(iClientPrunner->start(), false);
}
void ClientPluginRunnerTest::testSignals()
{
	QSignalSpy doneSignal(iClientPrunner, SIGNAL(done()));
	iClientPrunner->onThreadExit();
	QCOMPARE(doneSignal.count(), 1);
	
	QSignalSpy successSignal(iClientPrunner, SIGNAL(success(const QString &,
								const QString &)));
	iClientPrunner->onSuccess(PROFILE, "Message");
	QCOMPARE(successSignal.count(), 1);

	QSignalSpy errorSignal(iClientPrunner, SIGNAL(error(const QString &,
							    const QString &, int)));
	iClientPrunner->onError(PROFILE, "Message", 0);
	QCOMPARE(errorSignal.count(), 1);

	QSignalSpy transferSignal(iClientPrunner,
				  SIGNAL(transferProgress(const QString &,
							  Sync::TransferDatabase,
							  Sync::TransferType,
							  const QString &, int)));
	qRegisterMetaType<Sync::TransferDatabase>("Sync::TransferDatabase");
	qRegisterMetaType<Sync::TransferType>("Sync::TransferType");
	iClientPrunner->onTransferProgress(PROFILE, Sync::LOCAL_DATABASE,
					   Sync::ITEM_ADDED, "Mime",1);
	QCOMPARE(transferSignal.count(), 1);
}
TESTLOADER_ADD_TEST(ClientPluginRunnerTest);
