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
#include "SyncSchedulerTest.h"
#include "SyncScheduler.h"
#include "SyncProfile.h"
#include <ctime>
#include "SyncFwTestLoader.h"

using namespace Buteo;

void SyncSchedulerTest::init()
{
    iSyncScheduler = new SyncScheduler();
    iSyncProfileName.clear();
}

void SyncSchedulerTest::cleanup()
{
    if (iSyncScheduler)
    {
        delete iSyncScheduler;
        iSyncScheduler = NULL;
    }
}

void SyncSchedulerTest::syncTriggered(QString aProfileName)
{
    iSyncProfileName = aProfileName;
}

void SyncSchedulerTest::testAddRemoveProfile()
{
    SyncProfileStub profile("foo");
    profile.setEnabled(true);
    profile.setSyncType(SyncProfile::SYNC_SCHEDULED);

    bool profileAdded = iSyncScheduler->addProfile(&profile);
    QVERIFY(profileAdded);
    QVERIFY(iSyncScheduler->iSyncScheduleProfiles.contains(profile.name()));

    SyncProfileStub invalidProfile("bar");
    profile.setEnabled(false);
    profileAdded = iSyncScheduler->addProfile(&invalidProfile);
    QVERIFY(!profileAdded);

    iSyncScheduler->removeProfile("foo");
    QVERIFY(iSyncScheduler->iSyncScheduleProfiles.isEmpty());
}

void SyncSchedulerTest::testSetNextAlarm()
{
    QString profileName = "foo";
    SyncProfileStub profile(profileName);
    profile.setEnabled(true);
    profile.setSyncType(SyncProfile::SYNC_SCHEDULED);

    //int sanity = alarm_event_is_sane(iSyncScheduler->iAlarmEventParameters);
    //QVERIFY(sanity == 1);
    int alarmId = iSyncScheduler->setNextAlarm(&profile);
    QVERIFY(alarmId > 0);
    iSyncScheduler->removeAlarmEvent(alarmId);
}

TESTLOADER_ADD_TEST(SyncSchedulerTest);
