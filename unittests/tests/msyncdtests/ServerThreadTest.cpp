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
#include "ServerThreadTest.h"
#include "PluginManager.h"
#include "Logger.h"

using namespace Buteo;

const QString PGNAME = "Plugin";
const QString PFNAME = "Profile";
QString TYPE = Profile::TYPE_SERVER;

/* Defining the pure virtual functions of the base classes */
bool ServerPluginDerived::startListen()
{
	return true;
}

void ServerPluginDerived::stopListen()
{
}

void ServerPluginDerived::suspend()
{
}

void ServerPluginDerived::resume()
{
}

bool ServerPluginDerived::cleanUp()
{
	return true;
}

bool ServerPluginDerived::init()
{
	if (iTestSignal == true)
	{
		return false;
	}
	return true;
}
bool ServerPluginDerived::uninit()
{
	return true;
}
void ServerPluginDerived::connectivityStateChanged(Sync::ConnectivityType, bool)
{
}

/* Constructor of the serverPluginDerived Class */
ServerPluginDerived::ServerPluginDerived(const QString& aPluginName,
                                         const Profile& aProfile,
                                         PluginCbInterface* aCbInterface)
: ServerPlugin(aPluginName, aProfile, aCbInterface),
  iTestSignal(false)
{
}

void ServerThreadTest::initTestCase()
{
	iThreadreturn = false;
	
	iProfile = new Profile(PFNAME, TYPE);
	iServerThread = new ServerThread();
	iPluginDerived = new ServerPluginDerived(PGNAME, *iProfile, NULL);
	iThreadTestSp = iPluginDerived;
	
	//Logger::createInstance();
}
void ServerThreadTest::cleanupTestCase()
{
	QVERIFY(iProfile != 0);
	delete iProfile;
	QVERIFY(iPluginDerived != 0);
	delete iPluginDerived;
	QVERIFY(iServerThread != 0);
	delete iServerThread;
	
	iProfile = 0;
	iPluginDerived = 0;
	iServerThread = 0;
	iThreadTestSp = 0;

	//Logger::deleteInstance();
}
void ServerThreadTest::testServerThreadConstructor()
{
	QVERIFY(iServerThread->getPlugin() == NULL);
	QCOMPARE(iServerThread->iRunning, false);
}
void ServerThreadTest::testGetPlugin()
{
	iThreadreturn =iServerThread->startThread(iThreadTestSp);
	QTest::qWait(20);
	QCOMPARE(iThreadreturn, true);
	QCOMPARE(iServerThread->getPlugin(), iThreadTestSp);
}
void ServerThreadTest::testGetProfileName()
{
	QCOMPARE(iThreadreturn, true);
	QCOMPARE(iServerThread->getProfileName(), PFNAME);
}
void ServerThreadTest::testThread()
{
	//The Thread is already started in testGetPlugin()
	QCOMPARE(iServerThread->startThread(iThreadTestSp), false);
	QCOMPARE(iServerThread->iRunning, true);
	QCOMPARE(iServerThread->getPlugin(), iThreadTestSp);
}
void ServerThreadTest::testStopThErrorSignal()
{
	QVERIFY(iServerThread != 0);
	iServerThread->stopThread();
	iServerThread->wait(9000);
	iPluginDerived->iTestSignal = true;
	
	//Test for the signal
	QSignalSpy spy(iServerThread, SIGNAL(initError(const QString &,
												   const QString &, int)));
	QCOMPARE(iServerThread->startThread(iThreadTestSp), true);
	QTest::qWait(20);
	QCOMPARE(spy.count(), 1);
}

TESTLOADER_ADD_TEST(ServerThreadTest);

