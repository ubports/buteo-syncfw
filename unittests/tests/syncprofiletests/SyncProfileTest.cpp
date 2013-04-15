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
#include "SyncProfileTest.h"

#include <QDomDocument>
#include <QScopedPointer>

#include "SyncProfile.h"
#include "SyncFwTestLoader.h"
#include "ProfileEngineDefs.h"

using namespace Buteo;

static const QString NAME = "ovi-calendar";
static const QString TYPE = Profile::TYPE_SYNC;
static const QString PROFILE_XML =
    "<profile type=\"sync\" name=\"ovi-calendar\" >"
        "<key value=\"true\" name=\"enabled\" />"

        "<profile type=\"service\" name=\"ovi.com\" >"
            "<key value=\"me\" name=\"username\" />"
        "</profile>"

        "<profile type=\"client\" name=\"syncml\">"
        "</profile>"

        "<profile type=\"server\" name=\"syncmlserver\">"
        "</profile>"

        "<profile type=\"storage\" name=\"hcalendar\" >"
            "<key value=\"myNotebook\" name=\"Notebook Name\" />"
            "<key value=\"true\" name=\"enabled\" />"
            "<key value=\"cal-backend\" name=\"backend\" />"
        "</profile>"

        "<profile type=\"storage\" name=\"hcontacts\" />"

        "<schedule time=\"12:34:56\" interval=\"30\" days=\"1,2,3,4,5,6\">"
            "<rush enabled=\"true\" interval=\"15\""
                "begin=\"08:00:00\" end=\"16:00:00\" days=\"1,4,5\"/>"
        "</schedule>"
    "</profile>";

void SyncProfileTest::testConstruction()
{
    // Create from scratch.
    SyncProfile p1(NAME);
    QCOMPARE(p1.name(), NAME);
    QCOMPARE(p1.type(), TYPE);

    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(PROFILE_XML, false));
    SyncProfile p2(doc.documentElement());
    QCOMPARE(p2.name(), NAME);
    QCOMPARE(p2.type(), TYPE);
    QCOMPARE(p2.syncSchedule().interval(), (unsigned)30);

    // Copy constructor.
    QScopedPointer<SyncProfile> p3(p2.clone());
    QVERIFY(p3 != 0);
    QCOMPARE(p3->toString(), p2.toString());
}

void SyncProfileTest::testProperties()
{
    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(PROFILE_XML, false));
    SyncProfile p(doc.documentElement());

    // Get non-storage keys.
    QMap<QString, QString> keys = p.allNonStorageKeys();
    QCOMPARE(keys.size(), 2);
    QCOMPARE(keys.value("username"), QString("me"));
    QCOMPARE(keys.value("enabled"), QString("true"));

    // Get storage backend names.
    QStringList storages = p.storageBackendNames();
    QCOMPARE(storages.size(), 1);
    QCOMPARE(storages[0], QString("cal-backend"));

    // Change sync type.
    QCOMPARE(p.syncType(), SyncProfile::SYNC_MANUAL);
    p.setSyncType(SyncProfile::SYNC_SCHEDULED);
    QCOMPARE(p.syncType(), SyncProfile::SYNC_SCHEDULED);

    // Destination type.
    QCOMPARE(p.destinationType(), SyncProfile::DESTINATION_TYPE_UNDEFINED);
    //Profile *service = p.serviceProfile();
    //QVERIFY(service != 0);
    p.setKey(KEY_DESTINATION_TYPE, VALUE_ONLINE);
    QCOMPARE(p.destinationType(), SyncProfile::DESTINATION_TYPE_ONLINE);
    p.setKey(KEY_DESTINATION_TYPE, VALUE_DEVICE);
    QCOMPARE(p.destinationType(), SyncProfile::DESTINATION_TYPE_DEVICE);

    // Sync direction.
    QCOMPARE(p.syncDirection(), SyncProfile::SYNC_DIRECTION_UNDEFINED);
    Profile *client = p.clientProfile();
    QVERIFY(client != 0);
    client->setKey(KEY_SYNC_DIRECTION, VALUE_TWO_WAY);
    QCOMPARE(p.syncDirection(), SyncProfile::SYNC_DIRECTION_TWO_WAY);
    client->setKey(KEY_SYNC_DIRECTION, VALUE_FROM_REMOTE);
    QCOMPARE(p.syncDirection(), SyncProfile::SYNC_DIRECTION_FROM_REMOTE);
    client->setKey(KEY_SYNC_DIRECTION, VALUE_TO_REMOTE);
    QCOMPARE(p.syncDirection(), SyncProfile::SYNC_DIRECTION_TO_REMOTE);
    p.setSyncDirection(SyncProfile::SYNC_DIRECTION_TWO_WAY);
    QCOMPARE(client->key(KEY_SYNC_DIRECTION), VALUE_TWO_WAY);
    p.setSyncDirection(SyncProfile::SYNC_DIRECTION_FROM_REMOTE);
    QCOMPARE(client->key(KEY_SYNC_DIRECTION), VALUE_FROM_REMOTE);
    p.setSyncDirection(SyncProfile::SYNC_DIRECTION_TO_REMOTE);
    QCOMPARE(client->key(KEY_SYNC_DIRECTION), VALUE_TO_REMOTE);
    SyncProfile emptyProfile("empty");
    emptyProfile.setSyncDirection(SyncProfile::SYNC_DIRECTION_TWO_WAY);

    // Conflict resolution policy.
    QCOMPARE(p.conflictResolutionPolicy(), SyncProfile::CR_POLICY_UNDEFINED);
    QVERIFY(client != 0);
    client->setKey(KEY_CONFLICT_RESOLUTION_POLICY, VALUE_PREFER_REMOTE);
    QCOMPARE(p.conflictResolutionPolicy(), SyncProfile::CR_POLICY_PREFER_REMOTE_CHANGES);
    client->setKey(KEY_CONFLICT_RESOLUTION_POLICY, VALUE_PREFER_LOCAL);
    QCOMPARE(p.conflictResolutionPolicy(), SyncProfile::CR_POLICY_PREFER_LOCAL_CHANGES);
    p.setConflictResolutionPolicy(SyncProfile::CR_POLICY_PREFER_REMOTE_CHANGES);
    QCOMPARE(client->key(KEY_CONFLICT_RESOLUTION_POLICY), VALUE_PREFER_REMOTE);
    p.setConflictResolutionPolicy(SyncProfile::CR_POLICY_PREFER_LOCAL_CHANGES);
    QCOMPARE(client->key(KEY_CONFLICT_RESOLUTION_POLICY), VALUE_PREFER_LOCAL);
    emptyProfile.setConflictResolutionPolicy(SyncProfile::CR_POLICY_PREFER_REMOTE_CHANGES);
}

void SyncProfileTest::testResults()
{
    SyncProfile p(NAME);

    // Create log with some content.
    p.setLog(new SyncLog(NAME));
    QVERIFY(p.lastSyncTime().isNull());
    QDateTime now = QDateTime::currentDateTime();
    SyncResults syncResults(now, 1, 0);
    syncResults.addTargetResults(TargetResults("hcalendar",
        ItemCounts(1, 2, 3), ItemCounts()));
    p.addResults(syncResults);

    QCOMPARE(p.lastSyncTime(), now);
    const SyncResults *sr = p.lastResults();
    QVERIFY(sr != 0);
    QCOMPARE(sr->syncTime(), now);
    QCOMPARE(sr->majorCode(), 1);
    QCOMPARE(sr->targetResults().size(), 1);

    // 0 log.
    p.setLog(0);
    QVERIFY(p.lastResults() == 0);
    QVERIFY(p.lastSyncTime().isNull());
    p.addResults(syncResults);
    QCOMPARE(p.lastSyncTime(), now);
}

void SyncProfileTest::testNextSyncTime()
{
    // No next sync time in manual mode.
    SyncProfile p(NAME);
    QVERIFY(p.nextSyncTime(QDateTime::currentDateTime()).isNull());

    // Scheduled sync.
    p.setSyncType(SyncProfile::SYNC_SCHEDULED);
    QDateTime lastSync(QDateTime::currentDateTime());
    SyncResults r(lastSync, Buteo::SyncResults::SYNC_RESULT_SUCCESS, Buteo::SyncResults::NO_ERROR);
    p.addResults(r);
    SyncSchedule s;
    const unsigned INTERVAL = 15;
    s.setInterval(INTERVAL);
    DaySet days;
    days << Qt::Monday << Qt::Tuesday << Qt::Wednesday << Qt::Thursday <<
        Qt::Friday << Qt::Saturday << Qt::Sunday;
    s.setDays(days);
    p.setSyncSchedule(s);
    QDateTime nextSync = p.nextSyncTime(p.lastSyncTime());
    QCOMPARE(nextSync, lastSync.addSecs(INTERVAL * 60));
}

void SyncProfileTest::testSubProfiles()
{
    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(PROFILE_XML, false));
    SyncProfile p(doc.documentElement());

    const Profile *client = p.clientProfile();
    QVERIFY(client != 0);
    QVERIFY(client->name() == "syncml");

    //const Profile *service = p.serviceProfile();
    //QVERIFY(service != 0);
    QVERIFY(p.name() == "ovi.com");

    QList<const Profile*> storages = p.storageProfiles();
    QCOMPARE(storages.size(), 2);
    QList<Profile*> storages2 = p.storageProfilesNonConst();
    QCOMPARE(storages2.size(), 2);

    const Profile *server = p.serverProfile();
    QVERIFY(server != 0);
    QVERIFY(server->name() == "syncmlserver");

}

TESTLOADER_ADD_TEST(SyncProfileTest);
