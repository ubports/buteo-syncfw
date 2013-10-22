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
#include "SyncClientInterfaceTest.h"
#include "synchronizer.h"
#include "Profile.h"
#include "SyncSchedule.h"
#include "SyncProfile.h"
#include "SyncClientInterfacePrivate.h"

#include <QDebug>

using namespace Buteo;

static const QString USERPROFILE_DIR = "syncprofiletests/testprofiles/user";
static const QString SYSTEMPROFILE_DIR = "syncprofiletests/testprofiles/system";

void SyncClientInterfaceTest::initTestCase()
{
	iSync = new Synchronizer(NULL);

	iInterface = new SyncClientInterface();

	qRegisterMetaType<Buteo::Profile>("Buteo::Profile");
	qRegisterMetaType<Buteo::SyncResults>("Buteo::SyncResults");
}

void SyncClientInterfaceTest::cleanupTestCase()
{
    QVERIFY(iInterface != 0);
    delete iInterface;
    iInterface = 0;

    QVERIFY(iSync != 0);
    delete iSync;
    iSync = 0;
}


void SyncClientInterfaceTest::testIsValid()
{
	QCOMPARE(iInterface->isValid(),false);
}

void SyncClientInterfaceTest::testStartSync()
{
	QString empty("");
	QCOMPARE(iInterface->startSync(empty),false);

	QSignalSpy sigStatus(iInterface, SIGNAL(resultsAvailable(QString,Buteo::SyncResults)));

	QString profile("testsync-ovi");
	QCOMPARE(iInterface->startSync(profile),true);

/*	QCOMPARE(sigStatus.count(),1);*/
}

void SyncClientInterfaceTest::testAbortSync()
{
	QString profile("testsync-ovi");
	iInterface->abortSync(profile);
	QStringList list = iInterface->getRunningSyncList();
	sleep(1);
	QCOMPARE(list.size(),1);
}

void SyncClientInterfaceTest::testGetRunningSyncList()
{
	QString profile("testsync-ovi");
	iInterface->startSync(profile);
	QStringList list = iInterface->getRunningSyncList();
	QCOMPARE(list.size(),1);
}

void SyncClientInterfaceTest::testSetSyncSchedule()
{
	QString profile("testsync-ovi");
	Buteo::SyncSchedule schedule;
	QSignalSpy sigProfile(iInterface, SIGNAL(profileChanged(QString,int,Buteo::Profile)));
	QCOMPARE(iInterface->setSyncSchedule(profile,schedule),true);
}

void SyncClientInterfaceTest::testAddProfile()
{
	Buteo::SyncProfile profileToAdd("testsync-ovi");
	profileToAdd.setName("NewlyAddedProfile");
// 	QVERIFY(iInterface->addProfile(profileToAdd));
}

void SyncClientInterfaceTest::testUpdateProfile()
{
	Buteo::SyncProfile profileToChange("NewlyAddedProfile");
	QVERIFY(iInterface->updateProfile(profileToChange));
}

void SyncClientInterfaceTest::testRemoveProfile()
{
	Buteo::SyncProfile profileToChange("NewlyAddedProfile");
// 	QVERIFY(iInterface->addProfile(profileToChange));
}

TESTLOADER_ADD_TEST(SyncClientInterfaceTest);
