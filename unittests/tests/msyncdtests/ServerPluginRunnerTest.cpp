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

#include "ServerPluginRunnerTest.h"
#include <SyncCommonDefs.h>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include "SyncFwTestLoader.h"

using namespace Buteo;

void ServerPluginRunnerTest::initTestCase()
{
    iTransportTracker = new TransportTracker(this);
    iProfileManager = new ProfileManager("profile1", "profile2");
    iProfile = new Profile("dummyprofile", Profile::TYPE_SERVER);
    iProfileManager->updateProfile(*iProfile);

    iServerActivator = new ServerActivator(*iProfileManager, *iTransportTracker);

    QDir dir = QDir::current();
    QString path = dir.absolutePath();
    if (dir.cd("../dummyplugins/dummyserver"))
    {
        path = dir.absolutePath();
    } // no else

    iPluginManager = new PluginManager(path) ;

    // TODO: need to update with valid PluginCbInterface pointer
    iServerPluginRunner = new ServerPluginRunner("hdummy", iProfile, iPluginManager,
                                  reinterpret_cast<PluginCbInterface*>(this), iServerActivator);
    
    /* test init() */

    bool initResult = iServerPluginRunner->init();

    // after returning from init(), iPlugin should have the pointer to server thread and
    // iThread memory allocation should be success
    QVERIFY(iServerPluginRunner->plugin());
    QVERIFY(iServerPluginRunner->iThread);
    QVERIFY(initResult);

}

void ServerPluginRunnerTest::cleanupTestCase()
{
    // test ThreadStop()
    QVERIFY(iServerPluginRunner->iThread);
    iServerPluginRunner->stop();

    // deallocate allocated memory
    delete iServerPluginRunner;

    delete iPluginManager;
    delete iServerActivator;
    delete iProfileManager;
    delete iTransportTracker;

    /* don't deallocate memory for iProfile, as this will be deallocated by
     * ServerPluginRunner class. */
}

void ServerPluginRunnerTest::testStartAbort()
{
    /* testing start() */

    QVERIFY(iServerPluginRunner->iInitialized);
    bool isStarted = iServerPluginRunner->start();
    QVERIFY(isStarted);


    /* test abort() */

    // test plugin pointer for NULL
    QVERIFY(iServerPluginRunner->plugin());
    iServerPluginRunner->abort();

}

void ServerPluginRunnerTest::testSyncResults()
{
     // test plugin pointer for NULL
    QVERIFY(iServerPluginRunner->plugin());
    // we can't set values from ServerPluginRunner. Hence, comparing the default return value
    QCOMPARE(iServerPluginRunner->syncResults().majorCode(), 0);
}

void ServerPluginRunnerTest::testSignals()
{
    QSignalSpy sessionSpy(iServerPluginRunner, SIGNAL(newSession(QString)));
    QSignalSpy errorSpy(iServerPluginRunner, SIGNAL(error(QString, QString, int)));
    QSignalSpy successSpy(iServerPluginRunner, SIGNAL(success(QString, QString)));
    QSignalSpy doneSpy(iServerPluginRunner, SIGNAL(done()));

    // registering metatypes that are not known
    qRegisterMetaType<Sync::TransferDatabase>("Sync::TransferDatabase");
    qRegisterMetaType<Sync::TransferType>("Sync::TransferType");
    QSignalSpy transferSpy(iServerPluginRunner, SIGNAL(transferProgress(QString, Sync::TransferDatabase, Sync::TransferType, QString, int)));


    QVERIFY(iServerPluginRunner->iServerActivator);
    iServerPluginRunner->onNewSession("toDevice");
    QCOMPARE(sessionSpy.count(), 1);

    iServerPluginRunner->onTransferProgress("profile", Sync::LOCAL_DATABASE, Sync::ITEM_ADDED, "text", 1);
    QCOMPARE(transferSpy.count(), 1);

    iServerPluginRunner->onError("profile", "message", 2);
    QCOMPARE(errorSpy.count(), 1);

    iServerPluginRunner->onSuccess("profile", "message");
    QCOMPARE(successSpy.count(), 1);

    iServerPluginRunner->onThreadExit();
    QCOMPARE(doneSpy.count(), 1);

}


TESTLOADER_ADD_TEST(ServerPluginRunnerTest);
