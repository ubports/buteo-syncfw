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
#include "ClientThreadTest.h"
#include "SyncResults.h"

using namespace Buteo;

const QString PROFILE = "profile";
const QString PLUGIN = "plugin";
//QString TYPE = Profile::TYPE_CLIENT;

//Defining pure virtual functions of base classes
bool ClientPluginDerived::startSync()
{
	return true;
}
bool ClientPluginDerived::init()
{
	if(iTestClSignal == true)
	{
		return false;
	}
	return true;
}
bool ClientPluginDerived::uninit()
{
	return true;
}
void ClientPluginDerived::connectivityStateChanged(Sync::ConnectivityType ,bool)
{
}

bool ClientPluginDerived::cleanUp()
{
	return true;
}

//Constructor of the derived class
ClientPluginDerived::ClientPluginDerived(const QString& aPluginName,
                  	    		 const SyncProfile& aProfile,
                  	    		 PluginCbInterface* aCbInterface)
:ClientPlugin(aPluginName, aProfile, aCbInterface),
iTestClSignal(false)
{
}

void ClientThreadTest::initTestCase()
{
	iClientThread = new ClientThread();
	iSyncProfile = new SyncProfile(PROFILE);
	iPluginDerived = new ClientPluginDerived(PLUGIN, *iSyncProfile, NULL);
	iPlugin = iPluginDerived;
	iClientThreadRet = false;
}
void ClientThreadTest::cleanupTestCase()
{
	QVERIFY(iSyncProfile != 0);
	delete iSyncProfile;
	QVERIFY(iPluginDerived != 0);
	delete iPluginDerived;
	QVERIFY(iClientThread != 0);
	delete iClientThread;
	
	iSyncProfile = 0;
	iPluginDerived = 0;
	iClientThread = 0;
	iPlugin = 0;
}
void ClientThreadTest::testClientThreadConstructor()
{
	QVERIFY(iClientThread->getPlugin() == NULL);
	QCOMPARE(iClientThread->iRunning, false);
}
void ClientThreadTest::testGetPlugin()
{
	iClientThreadRet = iClientThread->startThread(iPlugin);
	QTest::qWait(20);
	QCOMPARE(iClientThreadRet, true);
	QCOMPARE(iClientThread->iRunning, true);
	QCOMPARE(iClientThread->getPlugin(), iPlugin);
}
void ClientThreadTest::testGetProfileName()
{
	QCOMPARE(iClientThreadRet, true);
	QCOMPARE(iClientThread->getProfileName(), PROFILE);
}
void ClientThreadTest::testClientThread()
{
	//The thread is already running
	QCOMPARE(iClientThread->startThread(iPlugin), false);
	QCOMPARE(iClientThread->iRunning, true);
}
void ClientThreadTest::testGetSyncResults()
{
	SyncResults syncRes;
	bool dateTime;
	QDateTime current;
	
	QVERIFY(iClientThread != 0);
	current = QDateTime::currentDateTime(); 
	iClientThread->stopThread();
	iClientThread->wait(9000);
	
	QCOMPARE(iClientThread->iRunning, false);
	syncRes = iClientThread->getSyncResults();
    QCOMPARE(syncRes.majorCode(), 0);
	QCOMPARE(syncRes.isScheduled(), false);
		
	dateTime = syncRes.syncTime() >= current;
	QVERIFY(dateTime);
	//QCOMPARE(syncRes.syncTime().date(), QDateTime::currentDateTime().date());
		
}
void ClientThreadTest::testInitError()
{
	iPluginDerived->iTestClSignal = true;
	
	//Test for the signal
	QSignalSpy spy(iClientThread, SIGNAL(initError(const QString &,
						       const QString &,
						       int )));
	QCOMPARE(iClientThread->startThread(iPlugin), true);
	QTest::qWait(20);
	QCOMPARE(spy.count(), 1);
}

TESTLOADER_ADD_TEST(ClientThreadTest);
