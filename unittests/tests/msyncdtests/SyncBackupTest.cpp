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
#include "SyncBackupTest.h"

#include <QDBusMessage>

using namespace Buteo;

void SyncBackupTest::initTestCase()
{
	iBackup = new SyncBackup();
}

void SyncBackupTest::cleanupTestCase()
{

    QVERIFY(iBackup != 0);  
    delete iBackup;   
    iBackup = 0;
}

void SyncBackupTest::testInitialize()
{
	QVERIFY(iBackup->iReply == 0);
	QVERIFY(iBackup->iBackupRestore == 0);
	QVERIFY(iBackup->iWatchService != 0);
	QVERIFY(iBackup->iAdaptor != 0);
}

void SyncBackupTest::testBackup()
{
	QDBusMessage msg;
	QSignalSpy sigStatus1(iBackup, SIGNAL(startBackup()));
	QCOMPARE(iBackup->backupStarts(msg), uchar(0));
	QCOMPARE(sigStatus1.count(), 1);
	QCOMPARE(iBackup->getBackUpRestoreState(), true); 
	
	QSignalSpy sigStatus2(iBackup, SIGNAL(backupDone()));
	QCOMPARE(iBackup->backupFinished(msg), uchar(0));
	QCOMPARE(sigStatus2.count(), 1);
	QCOMPARE(iBackup->getBackUpRestoreState(), false); 
	
	QSignalSpy sigStatus3(iBackup, SIGNAL(startRestore()));
	QCOMPARE(iBackup->restoreStarts(msg), uchar(0));
	QCOMPARE(sigStatus3.count(), 1);
	QCOMPARE(iBackup->getBackUpRestoreState(), true); 

	QSignalSpy  sigStatus4(iBackup, SIGNAL(restoreDone()));
	QCOMPARE(iBackup->restoreFinished(msg), uchar(0));
	QCOMPARE(sigStatus4.count(), 1);
	QCOMPARE(iBackup->getBackUpRestoreState(), false); 
}

QTEST_MAIN(Buteo::SyncBackupTest)
