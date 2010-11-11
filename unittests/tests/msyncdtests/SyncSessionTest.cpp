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



#include "SyncSessionTest.h"
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <SyncCommonDefs.h>

#include "SyncSession.h"
#include "PluginManager.h"
#include "PluginCbInterface.h"
#include "SyncFwTestLoader.h"
#include "SyncResults.h"

#include <QString>

using namespace Buteo;

bool SyncSessionTest  :: isValuePassedTrue;
int SyncSessionPluginRunnerTest :: testValue;

void SyncSessionTest :: init()
{
    iNullPluginRunner = 0;

    // initialize SyncSession
    iSyncProfile = new SyncProfile("foo");
    iSyncProfile->setEnabled(true);
    iSyncProfile->setSyncType(SyncProfile::SYNC_SCHEDULED);
    // TODO: need to include profile creation using xml

    // SyncSession pointer without iPluginRunner initialization
    iSyncSession = new SyncSession(iSyncProfile);

    // check whether the given profile is assigned to SyncSession->iProfile or not
    QCOMPARE(iSyncSession->profile(), iSyncProfile);


    // initialize iPluginRunner
    PluginManager samplePluginManager;

    iSyncSessionPluginRunnerTest = new SyncSessionPluginRunnerTest("testPlugin", &samplePluginManager, 0 );

}

void SyncSessionTest :: cleanup()
{
    // release the allocated memory

    // profile name will be deallocated in SyncSession. So, not deallocating
    delete iSyncSession ;
    iSyncSession = NULL;

    if(iSyncSessionPluginRunnerTest) {
        delete iSyncSessionPluginRunnerTest;
        iSyncSessionPluginRunnerTest = NULL;
    }
}


void SyncSessionTest :: testPluginRunner()
{

    /* testing setPluginRunner() & pluginRunner() */

    // set iPluginRunner to a value and check whether the same exists in iPluginRunner

    /* by passing a valid PluginRunner pointer to SyncSession, the class should be able to
     * register with the signals successfully. If it not a valid PluginRunner pointer,
     * if should throw warnings
     */

    iSyncSession->setPluginRunner(iSyncSessionPluginRunnerTest, true);
    QCOMPARE(iSyncSession->iPluginRunnerOwned, true);
    QCOMPARE(iSyncSession->pluginRunner(), iSyncSessionPluginRunnerTest);

    // Check whether the same value is set to iPluginRunner or not
    QCOMPARE(iSyncSession->iPluginRunner, iSyncSession->pluginRunner());

    iSyncSession->onDestroyed(iSyncSessionPluginRunnerTest);
    QCOMPARE(iSyncSession->pluginRunner(), iNullPluginRunner);



    /* set NULL to iPluginRunner and test */

    iSyncSession->setPluginRunner(iNullPluginRunner, false);
    QCOMPARE(iSyncSession->iPluginRunnerOwned, false);
    QCOMPARE(iSyncSession->iPluginRunner, iNullPluginRunner);

    // Check whether the same value is set to iPluginRunner or not
    QCOMPARE(iSyncSession->iPluginRunner, iSyncSession->pluginRunner());

    iSyncSession->onDestroyed(iSyncSessionPluginRunnerTest);
    QCOMPARE(iSyncSession->pluginRunner(), iNullPluginRunner);
}


void SyncSessionTest :: testProfile()
{
    /* testing profile() and profileName()   */

    const QString PROFILENAME ="Profile";

    iSyncSession->iProfile->setName(PROFILENAME);
    SyncProfile *sampleProfile = iSyncSession->profile();
    QCOMPARE(PROFILENAME, sampleProfile->name());

    QString profileName = iSyncSession->profileName();

    QVERIFY(profileName != 0);
    QCOMPARE(iSyncSession->profileName(), iSyncSession->iProfile->name());

}

void SyncSessionTest :: testStartAbortStop()
{
    bool isStarted;

    // testing with iNullPluginRunner

    iSyncSession->setPluginRunner(iNullPluginRunner, true);
    QCOMPARE(iNullPluginRunner, iSyncSession->pluginRunner());

    /* testing start() */

    isStarted = iSyncSession->start();
    QCOMPARE(isStarted, false);

    /* testing abort() */
    iSyncSession->abort();
    QVERIFY(iSyncSession->iAborted);

    // stop() neither updates nor calls other functions for iPluginRunner NULL value. so, omitting this function for NULL value


    // testing with iSyncSessionPluginRunnerTest

    iSyncSession->setPluginRunner(iSyncSessionPluginRunnerTest, true);
    QCOMPARE(iSyncSessionPluginRunnerTest, iSyncSession->iPluginRunner);

    /* testing start() */

    // test start() with both possible values
    isValuePassedTrue = false;
    isStarted = iSyncSession->start();
    QCOMPARE(isStarted, isValuePassedTrue);

    isValuePassedTrue = true;
    isStarted = iSyncSession->start();
    QCOMPARE(isStarted, isValuePassedTrue);

    /* testing abort() */
    iSyncSession->abort();
    QCOMPARE(SyncSessionPluginRunnerTest::testValue, 2);

    iSyncSession->stop();
    QCOMPARE(SyncSessionPluginRunnerTest::testValue, 3);

}

void SyncSessionTest :: testResults()
{
    SyncResults results1;

    results1.setMajorCode(Buteo::SyncResults::SYNC_RESULT_CANCELLED);
    iSyncSession->updateResults(results1);

    SyncResults results2 = iSyncSession->results();

    QCOMPARE(results1.majorCode(), results2.majorCode());

    // testing setFailureResult()
    iSyncSession->setFailureResult(Buteo::SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);

    SyncResults givenResult = iSyncSession->results();
    SyncResults receivedtResult = iSyncSession->results();

    QCOMPARE(givenResult.majorCode(), receivedtResult.majorCode());

}

void SyncSessionTest :: testScheduled()
{
    /* testing both setScheduled() & isScheduled() */

    bool givenScheduledData = false;

    iSyncSession->setScheduled(givenScheduledData);
    QCOMPARE(iSyncSession->isScheduled(), givenScheduledData);
    
    givenScheduledData = true;

    iSyncSession->setScheduled(givenScheduledData);
    QCOMPARE(iSyncSession->isScheduled(), givenScheduledData);
}

void SyncSessionTest :: testStorages()
{
    // TODO: need to check before and after calling reserveStorage()
    // testing reserveStorages()
    StorageBooker *myTestStorageBooker = new StorageBooker ;
    QString oneStorage;

    foreach (QString storage, iSyncProfile->storageBackendNames())
    {
        // take one name of storage to check the storage
        oneStorage = storage;
        break;
    }
    QVERIFY(myTestStorageBooker->isStorageAvailable(oneStorage, iSyncProfile->name()));

    bool isSuccess = iSyncSession->reserveStorages(myTestStorageBooker);
    QCOMPARE(iSyncSession->iStorageBooker, myTestStorageBooker);
    QVERIFY(isSuccess);
    
    
    QVERIFY(myTestStorageBooker->isStorageAvailable(oneStorage, iSyncProfile->name()));

    //testing releaseStorages()
    // releaseStorages() can be verified
    iSyncSession->releaseStorages();
    QVERIFY(myTestStorageBooker->isStorageAvailable(oneStorage, iSyncProfile->name()));

    if(myTestStorageBooker) {
        delete myTestStorageBooker;
        myTestStorageBooker = NULL;
    }
}

void SyncSessionTest :: testOnSuccess()
{
    const QString PROFILE = "sampleProfile";
    const QString MESSAGE  = "testMessage";

    iSyncSession->setPluginRunner(iSyncSessionPluginRunnerTest, true);
    iSyncSession->onSuccess(PROFILE, MESSAGE);
    QCOMPARE(iSyncSession->iMessage, MESSAGE);
    QVERIFY(iSyncSession->iFinished);

    // testing iStatus when onSuccess() is called
    iSyncSession->iAborted = false;
    iSyncSession->onSuccess(PROFILE, MESSAGE);
    QCOMPARE(iSyncSession->iStatus, Sync::SYNC_DONE);
    iSyncSession->iAborted = true;
    iSyncSession->onSuccess(PROFILE, MESSAGE);
    QCOMPARE(iSyncSession->iStatus, Sync::SYNC_ABORTED);

    isValuePassedTrue = iSyncSession->isScheduled();
    QCOMPARE(iSyncSession->iPluginRunner->syncResults().majorCode(), iSyncSession->iResults.majorCode());
    QCOMPARE(iSyncSession->isScheduled(), iSyncSession->iResults.isScheduled());

    // finished() signal is checked in testOnDone()

}

void SyncSessionTest :: testOnError()
{
    const QString MESSAGE = "testMessage";
    int errorCode = 4;

    iSyncSession->setPluginRunner(iSyncSessionPluginRunnerTest, true);
    iSyncSession->onError("sampleProfile", MESSAGE, errorCode);
    QCOMPARE(iSyncSession->iMessage, MESSAGE);
    QCOMPARE(iSyncSession->iErrorCode, errorCode);
    QCOMPARE(iSyncSession->iStatus, Sync::SYNC_ERROR);
    QVERIFY(iSyncSession->iFinished);

    isValuePassedTrue = iSyncSession->iScheduled;
    QCOMPARE(iSyncSession->iPluginRunner->syncResults().majorCode(), iSyncSession->iResults.majorCode());
    QCOMPARE(iSyncSession->results().isScheduled(), iSyncSession->iResults.isScheduled());

}

void SyncSessionTest ::  testOnTransferProgress()
{
    // registering metatypes that are not known
    qRegisterMetaType<Sync::TransferDatabase>("Sync::TransferDatabase");
    qRegisterMetaType<Sync::TransferType>("Sync::TransferType");

    QSignalSpy sampleSpy(iSyncSession, SIGNAL(transferProgress(QString, Sync::TransferDatabase, Sync::TransferType,QString,int)));

    QVERIFY(iSyncSessionPluginRunnerTest);

    // call functions that emit transferProgress() signal

    iSyncSession->onTransferProgress("profile", Sync::LOCAL_DATABASE, Sync::ITEM_ADDED, "text", 1);
    QCOMPARE(sampleSpy.count(), 1);
}

void SyncSessionTest :: testOnDone()
{
    // registering unknown metatype
    qRegisterMetaType<Sync::SyncStatus>("Sync::SyncStatus");

    QSignalSpy sampleSpy(iSyncSession, SIGNAL(finished(QString, Sync::SyncStatus,QString, int)));

    // call functions that emit finished() signal
    iSyncSession->onDone();
    QCOMPARE(sampleSpy.count(), 1);

    int errorCode = 1;

    iSyncSession->onSuccess("testProfile", "testMessage");
    QCOMPARE(sampleSpy.count(), 2);

    iSyncSession->onError("testProfile", "testMessage", errorCode);
    QCOMPARE(sampleSpy.count(), 3);
}


// ############################################
/*
 * Starting SyncSessionPluginRunnerTest class
 */
// ############################################


SyncSessionPluginRunnerTest :: SyncSessionPluginRunnerTest(const QString &aPluginName,
                             PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf)
         : PluginRunner(PLUGIN_CLIENT, aPluginName, aPluginMgr, aPluginCbIf)
{

}

bool SyncSessionPluginRunnerTest :: init()
{
    /* Checking whether control is coming to PluginRuneer derived class or not */

    if(SyncSessionTest::isValuePassedTrue )
        return true;
    else
        return false;    
}

bool SyncSessionPluginRunnerTest :: start()
{
    /* Checking whether control is coming to PluginRuneer derived class or not */

    if(SyncSessionTest::isValuePassedTrue )
        return true;
    else
        return false;

}

void SyncSessionPluginRunnerTest ::stop ()
{
    // check the value after returning to the calling function

    testValue = 3;
}

void SyncSessionPluginRunnerTest :: abort()
{
    // check the value after returning to the calling function

    testValue = 2;
}

SyncResults SyncSessionPluginRunnerTest :: syncResults()
{
    SyncResults results;
    results.setScheduled(SyncSessionTest::isValuePassedTrue);

    return results;

}

SyncPluginBase* SyncSessionPluginRunnerTest :: plugin()
{
    // This is not being used by SyncSession. returning NULL to supress compile warning

    return (SyncPluginBase*)0;
}

bool SyncSessionPluginRunnerTest :: cleanUp()
{
	// check the value after returning to the calling function
	return true;
}

//TESTLOADER_ADD_TEST(SyncSessionTest);
