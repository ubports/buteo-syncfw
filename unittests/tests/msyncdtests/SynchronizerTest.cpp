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
#include "SynchronizerTest.h"
#include "Logger.h"
#include "TransportTracker.h"
#include "ServerActivator.h"
#include "ServerPluginRunner.h"


using namespace Buteo;

void SynchronizerTest::initTestCase()
{
	iSync = new Synchronizer(NULL);
	iProfile = new SyncProfile("Profile");
	iProfileQ = new SyncProfile("Profile1");
	iSyncSession = new SyncSession(iProfile, NULL);
	iSyncSessionQ = new SyncSession(iProfileQ, NULL);
	
    //Logger::createInstance();
}
void SynchronizerTest::cleanupTestCase()
{

	iSync->close();
	
	QVERIFY(iSyncSession != 0);
	delete iSyncSession;	
	QVERIFY(iSyncSessionQ != 0);
	delete iSyncSessionQ;

    QVERIFY(iSync != 0);
    iSync->close();
    delete iSync;
	
	iSync = 0;
	iSyncSession = 0;
	iSyncSessionQ = 0;
    //Logger::deleteInstance();
}
void SynchronizerTest::testSyncConstructor()
{
	QVERIFY(iSync->iSyncScheduler == 0);
	QVERIFY(iSync->iTransportTracker == 0);
	QVERIFY(iSync->iServerActivator == 0);
	QVERIFY(iSync->iAccounts == 0);
	QVERIFY(iSync->iSyncBackup == 0);
	QCOMPARE(iSync->iClosing, false);
}
void SynchronizerTest::testInitialize()
{
	QCOMPARE(iSync->isConnectivityAvailable(Sync::CONNECTIVITY_USB), false);
	QCOMPARE(iSync->initialize(), true);
	QVERIFY(iSync->iSyncScheduler != 0);
	QVERIFY(iSync->iTransportTracker != 0);
	QVERIFY(iSync->iServerActivator != 0);
	QVERIFY(iSync->iAccounts != 0);
	QVERIFY(iSync->iSyncBackup != 0);
	QVERIFY(iSync->iLowPower != 0);
}
void SynchronizerTest::testSync()
{
	QString name;
	QStringList alist;
	alist << "storage";
	SyncSession *aSync = 0;

	QVERIFY(aSync == 0);
	QVERIFY(iSync->iSyncBackup != 0);
	QCOMPARE(iSync->startSyncNow(aSync), false);

	QVERIFY(iSyncSession != 0);
	QCOMPARE(iSync->startSyncNow(iSyncSession), false);

	QCOMPARE(iSync->startSync("Profile"), false);
	QCOMPARE(iSync->startScheduledSync("Profile"), false);
	iSync->iActiveSessions.insert(iSyncSession->profileName(), iSyncSession);
	QCOMPARE(iSync->startSync("Profile"), true);
	QCOMPARE(iSync->startScheduledSync("Profile"), true);
	QCOMPARE(iSync->runningSyncs().takeFirst(), iSyncSession->profileName());

	QCOMPARE(iSync->startSync("Profile1"), false);
	QCOMPARE(iSync->startScheduledSync("Profile1"), false);
	iSync->iSyncQueue.enqueue(iSyncSessionQ);
	QSignalSpy sigStatus(iSync, SIGNAL(syncStatus(QString, int, QString, int)));
	QCOMPARE(iSync->startSync("Profile1"), true);
	QCOMPARE(sigStatus.count(), 1);
	
	QSignalSpy sigStatus1(iSync, SIGNAL(syncStatus(QString, int, QString, int)));
	QCOMPARE(iSync->startScheduledSync("Profile1"), true);
	QCOMPARE(sigStatus1.count(), 1);

	QVERIFY(iSync->iLowPower != 0);
	QCOMPARE(iSync->startSync("S40"), false);
	QCOMPARE(iSync->requestStorages(alist), true);

	//test startNextSync()
	QCOMPARE(iSync->iSyncQueue.isEmpty(), false);
	QVERIFY(iSync->iSyncQueue.head() != 0);
	QCOMPARE(iSync->startNextSync(), true);
	QCOMPARE(iSync->iSyncQueue.isEmpty(), true);
	QCOMPARE(iSync->startNextSync(), false);

	//test abortSync()
	iSync->iActiveSessions.insert(iSyncSession->profileName(), iSyncSession);
	QCOMPARE(iSync->iActiveSessions.contains("Profile"), true);
	iSync->abortSync("Profile");

	//test createStorage(const QString)
	QCOMPARE(QString("Plugin").isEmpty(), false);
	QVERIFY(iSync->createStorage("Plugin") == NULL);

	QCOMPARE(iSync->requestStorage("Storage", reinterpret_cast<SyncPluginBase*>(this)), true);

	//Test startServer(const QString &aProfileName)
	QVERIFY(iSync->iServerActivator != 0);
	QCOMPARE(iSync->iServers.isEmpty(), true);
	QVERIFY(iSync->iProfileManager.profile("profile", Profile::TYPE_SERVER) == 0);
	iSync->startServer("profile");
	QCOMPARE(iSync->iServers.isEmpty(), true);
	QVERIFY(iSync->iProfileManager.profile("syncml", Profile::TYPE_SERVER) != 0);
	iSync->startServer("syncml");
    QCOMPARE(iSync->iServers.isEmpty(), false);
    QTest::qSleep( 100 );

    // Fake that synchronizer is closing, so that server plug-in is deleted
    // immediately. Otherwise plug-in will get deleted only after synchronizer
    // destructor when plug-in manager is already destroyed. This will cause a
    // a crash because plug-in runner uses the plug-in manager to destroy the
    // plug-in. In the real application this situation does not happen, because
    // event loop is running and deleteLater works normally.
    iSync->iClosing = true;
	iSync->stopServer("syncml");
    iSync->iClosing = false;

	QCOMPARE(iSync->iServers.isEmpty(), true);

	QCOMPARE(iSync->iServerActivator->enabledServers().isEmpty(), true);

	QVERIFY(iSync->iTransportTracker != 0);
	iSync->iTransportTracker->updateState(Sync::CONNECTIVITY_USB, true);

	//test startServers()
	QVERIFY(iSync->iServerActivator != 0);
	QCOMPARE(iSync->iServerActivator->enabledServers().isEmpty(), false);
	iSync->startServers();
    QTest::qSleep( 100 );

    // Fake that synchronizer is closing, so that server plug-in is deleted
    // immediately.
    iSync->iClosing = true;
	iSync->stopServers();
    iSync->iClosing = false;

	//test onServerDone()
	//This is not called in a slot activated by the signal. sender() returns 0
	QVERIFY(iSync->iProfileManager.profile("syncml", Profile::TYPE_SERVER) != 0);
	iSync->startServer("syncml");
    QTest::qSleep( 100 );
	QCOMPARE(iSync->iServers.isEmpty(), false);
	QCOMPARE(iSync->iServers.size(), 1);
	iSync->onServerDone();
	QCOMPARE(iSync->iServers.size(), 1);

	//test onNewSession(const QString &aDestination).
	//This is not called in a slot activated by the signal. sender() returns 0
	QCOMPARE(iSync->iActiveSessions.isEmpty(), false);
	QCOMPARE(iSync->iActiveSessions.size(), 1);
	iSync->onNewSession("USB");
	QCOMPARE(iSync->iActiveSessions.size(), 1);

    /*
    ServerPluginRunner *plugin = new ServerPluginRunner("Plugin", NULL, NULL, NULL, NULL);
	QSignalSpy spy(plugin, SIGNAL(newSession(const QString )));
	plugin->onNewSession("USB");
	QCOMPARE(spy.count(), 1);
	QTest::qWait(20);
    QCOMPARE(iSync->iActiveSessions.size(), 1);*/

	QVERIFY(iSync->iSyncScheduler != 0);
	iSync->reschedule("Profile");

	//test is TransportAvailable(const SyncSession *aSession)
	QVERIFY(iSyncSession != 0);
	QVERIFY(iSync->iTransportTracker != 0);
	QVERIFY(iSyncSession->profile() != 0);
}
void SynchronizerTest::testSignals()
{
	QStringList alist;
	alist << "storage";

	QSignalSpy sigStorage(iSync, SIGNAL(storageReleased()));
	iSync->releaseStorages(alist);
	QCOMPARE(sigStorage.count(), 1);
	
	qRegisterMetaType<Sync::TransferDatabase>("Sync::TransferDatabase");
	qRegisterMetaType<Sync::TransferType>("Sync::TransferType");
	QSignalSpy sigTransfer(iSync,
			       SIGNAL(transferProgress(QString, int, int, QString, int)));
	
	iSync->onTransferProgress("Profile", Sync::LOCAL_DATABASE,
				  Sync::ITEM_ADDED, "Mime", 1);
	QCOMPARE(sigTransfer.count(), 1);

	QSignalSpy sigStorage1(iSync, SIGNAL(storageReleased()));
	iSync->releaseStorage("Storage", NULL);
	QCOMPARE(sigStorage1.count(), 1);

	
	QSignalSpy sigbackupRestoreStarts(iSync, SIGNAL(backupInProgress()));
	iSync->backupStarts();
	QCOMPARE(sigbackupRestoreStarts.count(), 1);
	
	QSignalSpy sigbackupDone(iSync, SIGNAL(backupDone()));
	iSync->backupFinished();
	QCOMPARE(sigbackupDone.count(), 1);
	
	QSignalSpy sigrestoreStarts(iSync, SIGNAL(restoreInProgress()));
	iSync->restoreStarts();
	QCOMPARE(sigrestoreStarts.count(), 1);
	
	QSignalSpy sigrestoreFinished(iSync, SIGNAL(restoreDone()));
	iSync->restoreFinished();
	QCOMPARE(sigrestoreFinished.count(), 1);
	
	QSignalSpy sessionStatus(iSync, SIGNAL(syncStatus(QString, int, QString, int)));
	iSync->onSessionFinished("Profile", Sync::SYNC_DONE, "Msg", 0);
	QCOMPARE(sessionStatus.count(), 1);
}
TESTLOADER_ADD_TEST(SynchronizerTest);
