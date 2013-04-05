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
#include "ProfileManagerTest.h"
#include "ProfileManager.h"
#include "ProfileEngineDefs.h"
#include "SyncFwTestLoader.h"
#include "StorageProfile.h"
#include "SyncResults.h"

#include <Logger.h>
#include <QScopedPointer>
#include <QFile>

using namespace Buteo;

static const QString HCALENDAR = "hcalendar";
static const QString OVI_CALENDAR = "ovi-calendar";
static const QString CORRUPTED = "corrupted";
static const QString OVI_COM = "ovi.com";
static const QString SYNCML = "syncml";
static const QString USERPROFILE_DIR = "syncprofiletests/testprofiles/user";
static const QString SYSTEMPROFILE_DIR = "syncprofiletests/testprofiles/system";


void ProfileManagerTest::initTestCase()
{
    //Logger::createInstance();
}


void ProfileManagerTest::cleanupTestCase()
{
    //Logger::deleteInstance();
}


void ProfileManagerTest::testGetProfile()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);

    // Storage profiles exist.
    QStringList names = pm.profileNames(Profile::TYPE_STORAGE);
    QVERIFY(!names.isEmpty());
    QVERIFY(names.contains(HCALENDAR));

    // No profiles for unknown type.
    QVERIFY(pm.profileNames("unknown").isEmpty());

    // Getting unknown profile returns 0.
    QVERIFY(pm.profile("unknown", Profile::TYPE_SYNC) == 0);

    // Get a storage profile.
    QScopedPointer<Profile> p(pm.profile(HCALENDAR, Profile::TYPE_STORAGE));
    QVERIFY(p != 0);
    QCOMPARE(p->name(), HCALENDAR);
    QCOMPARE(p->type(), Profile::TYPE_STORAGE);
    QVERIFY(dynamic_cast<StorageProfile*>(p.data()) != 0);

    // Getting unknown sync profile returns 0.
    QVERIFY(pm.syncProfile("unknown") == 0);

    // Get a sync profile.
    QScopedPointer<Profile> sp(pm.profile(OVI_CALENDAR, Profile::TYPE_SYNC));
    QVERIFY(sp != 0);
    QCOMPARE(sp->name(), OVI_CALENDAR);
    QCOMPARE(sp->type(), Profile::TYPE_SYNC);

    // No client sub-profile before expanding.
    QCOMPARE(sp->isLoaded(), false);
    QVERIFY(sp->subProfile(SYNCML, Profile::TYPE_CLIENT) == 0);

    // Client sub-profile available after expanding.
    pm.expand(*sp);
    QCOMPARE(sp->isLoaded(), true);
    Profile *sub = sp->subProfile(SYNCML, Profile::TYPE_CLIENT);
    QVERIFY(sub != 0);
    QCOMPARE(sub->isLoaded(), true);
    QCOMPARE(sub->name(), SYNCML);
    QCOMPARE(sub->type(), Profile::TYPE_CLIENT);

    // Key merged from sub-sub-profile.
    sub = sp->subProfile(HCALENDAR, Profile::TYPE_STORAGE);
    QVERIFY(sub != 0);
    QCOMPARE(sub->key("Local URI"), QString("./Calendar"));
}

void ProfileManagerTest::testGetSyncProfile()
{
    ProfileManager pm(USERPROFILE_DIR + '/', SYSTEMPROFILE_DIR + '/');

    // Getting unknown sync profile returns 0.
    QVERIFY(pm.syncProfile("unknown") == 0);

    // Get a sync profile.
    QScopedPointer<SyncProfile> sp(pm.syncProfile(OVI_CALENDAR));
    QVERIFY(sp != 0);
    QCOMPARE(sp->name(), OVI_CALENDAR);
    QCOMPARE(sp->type(), Profile::TYPE_SYNC);

    // Sync profile is expanded automatically.
    QCOMPARE(sp->isLoaded(), true);
    Profile *sub = sp->subProfile(SYNCML, Profile::TYPE_CLIENT);
    QVERIFY(sub != 0);
    sub = sp->subProfile(HCALENDAR, Profile::TYPE_STORAGE);
    QVERIFY(sub != 0);
    QCOMPARE(sub->key("Local URI"), QString("./Calendar"));

    // Get all sync profiles.
    QList<SyncProfile*> allProfiles = pm.allSyncProfiles();
    QVERIFY(!allProfiles.isEmpty());
    QCOMPARE(allProfiles.first()->name(), OVI_CALENDAR);
    QCOMPARE(allProfiles.first()->type(), Profile::TYPE_SYNC);
    foreach (SyncProfile *p, allProfiles)
    {
        delete p;
    }
    allProfiles.clear();
}

void ProfileManagerTest::testGetByData()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);
    QList<SyncProfile*> profiles;

    // Get profiles with non-existent key.
    profiles = pm.getSyncProfilesByData("", "", "unknown");
    QVERIFY(profiles.isEmpty());

    // Get profiles with existing key, defined value, no match.
    profiles = pm.getSyncProfilesByData("", "", "enabled", "false");
    QVERIFY(profiles.isEmpty());

    // Get profiles with existing key, undefined value.
    profiles = pm.getSyncProfilesByData("", "", "enabled");
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles with existing key, defined value, match.
    profiles = pm.getSyncProfilesByData("", "", "enabled", "true");
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->key("enabled"), QString("true"));
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, no match.
    profiles = pm.getSyncProfilesByData("unknown", Profile::TYPE_SYNC);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile information, key and value defined, no match.
    profiles = pm.getSyncProfilesByData(OVI_COM, Profile::TYPE_SYNC,
        "Username", "unknown");
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile information, sub-profile name not defined,
    // key and value defined, no match.
    profiles = pm.getSyncProfilesByData("", Profile::TYPE_SYNC,
        "Username", "unknown");
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile information, no key defined.
    profiles = pm.getSyncProfilesByData(OVI_COM, Profile::TYPE_SYNC);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, key and value defined.
    profiles = pm.getSyncProfilesByData(OVI_COM, Profile::TYPE_SYNC,
        "Username", "your_username");
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, sub-profile name not defined,
    // key and value defined.
    profiles = pm.getSyncProfilesByData("", Profile::TYPE_SYNC,
        "Username", "your_username");
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();
}

void ProfileManagerTest::testGetBySingleCriteria()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);
    QList<SyncProfile*> profiles;

    // Get profiles with non-existent key.
    ProfileManager::SearchCriteria criteria;
    QList<ProfileManager::SearchCriteria> criteriaList;
    criteria.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria.iKey = "unknown";
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles with existing key, defined value, no match.
    criteria.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria.iKey = "enabled";
    criteria.iValue = "false";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles with existing key, undefined value.
    criteria.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria.iKey = "enabled";
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles with existing key, defined value, match.
    criteria.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria.iKey = "enabled";
    criteria.iValue = "true";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->key("enabled"), QString("true"));
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, no match.
    criteria.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria.iSubProfileName = "unknown";
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = QString::null;
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile, no match.
    criteria.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria.iSubProfileName = "hsms";
    criteria.iSubProfileType = Profile::TYPE_STORAGE;
    criteria.iKey = QString::null;
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile.
    criteria.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria.iSubProfileName = HCALENDAR;
    criteria.iSubProfileType = Profile::TYPE_STORAGE;
    criteria.iKey = QString::null;
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile, no match.
    criteria.iType = ProfileManager::SearchCriteria::NOT_EXISTS;
    criteria.iSubProfileName = HCALENDAR;
    criteria.iSubProfileType = Profile::TYPE_STORAGE;
    criteria.iKey = QString::null;
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile.
    criteria.iType = ProfileManager::SearchCriteria::NOT_EXISTS;
    criteria.iSubProfileName = "hsms";
    criteria.iSubProfileType = Profile::TYPE_STORAGE;
    criteria.iKey = QString::null;
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, key and value defined, no match.
    criteria.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria.iSubProfileName = OVI_COM;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = "Username";
    criteria.iValue = "unknown";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile information, sub-profile name not defined,
    // key and value defined, no match.
    criteria.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria.iSubProfileName = QString::null;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = "Username";
    criteria.iValue = "unknown";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile information, no key defined.
    criteria.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria.iSubProfileName = OVI_COM;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = QString::null;
    criteria.iValue = QString::null;
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, key and value defined.
    criteria.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria.iSubProfileName = OVI_COM;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = "Username";
    criteria.iValue = "your_username";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, key and value defined, no match.
    criteria.iType = ProfileManager::SearchCriteria::NOT_EQUAL;
    criteria.iSubProfileName = OVI_COM;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = "Username";
    criteria.iValue = "your_username";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Get profiles by sub-profile information, key and value defined.
    criteria.iType = ProfileManager::SearchCriteria::NOT_EQUAL;
    criteria.iSubProfileName = OVI_COM;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = "Username";
    criteria.iValue = "foobar";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Get profiles by sub-profile information, sub-profile name not defined,
    // key and value defined.
    criteria.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria.iSubProfileName = QString::null;
    criteria.iSubProfileType = Profile::TYPE_SYNC;
    criteria.iKey = "Username";
    criteria.iValue = "your_username";
    criteriaList.clear();
    criteriaList.append(criteria);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();
}

void ProfileManagerTest::testGetByMultipleCriteria()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);
    QList<SyncProfile*> profiles;

    QList<ProfileManager::SearchCriteria> criteriaList;

    // Profile must be enabled.
    ProfileManager::SearchCriteria criteria1;
    criteria1.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria1.iKey = KEY_ENABLED;
    criteria1.iValue = BOOLEAN_TRUE;
    criteriaList.append(criteria1);

    // Profile must not be hidden.
    ProfileManager::SearchCriteria criteria2;
    criteria2.iType = ProfileManager::SearchCriteria::NOT_EQUAL;
    criteria2.iKey = KEY_HIDDEN;
    criteria2.iValue = BOOLEAN_TRUE;
    criteriaList.append(criteria2);

    // Profile must have a hcalendar storage.
    ProfileManager::SearchCriteria criteria3;
    criteria3.iType = ProfileManager::SearchCriteria::EXISTS;
    criteria3.iSubProfileName = "hcalendar";
    criteria3.iSubProfileType = Profile::TYPE_STORAGE;
    criteriaList.append(criteria3);

    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();

    // Add a fourth criteria, that doesn't match to any profile.
    ProfileManager::SearchCriteria criteria4;
    criteria4.iType = ProfileManager::SearchCriteria::EQUAL;
    criteria4.iSubProfileName = "hcalendar";
    criteria4.iSubProfileType = Profile::TYPE_STORAGE;
    criteria4.iKey = "Notebook Name";
    criteria4.iValue = "Personal";
    criteriaList.append(criteria4);
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(profiles.isEmpty());

    // Replace the fourth criteria with a criteria that matches.
    criteria4.iValue = "myNotebook";
    criteriaList[3] = criteria4;
    profiles = pm.getSyncProfilesByData(criteriaList);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();
}

void ProfileManagerTest::testGetByStorage()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);
    QList<SyncProfile*> profiles;

    // Get profiles by storage.
    profiles = pm.getSyncProfilesByStorage("hcalendar", true);
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles[0] != 0);
    QCOMPARE(profiles[0]->name(), OVI_CALENDAR);
    foreach (SyncProfile *p, profiles)
    {
        delete p;
    }
    profiles.clear();
}

void ProfileManagerTest::testLog()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);

    {
        QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
        QVERIFY(p != 0);

        // Create log with some content.
        p->setLog(new SyncLog(OVI_CALENDAR));
        QDateTime now = QDateTime::currentDateTime();
        SyncResults syncResults(now, Buteo::SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
        syncResults.addTargetResults(TargetResults("hcalendar",
            ItemCounts(1, 2, 3), ItemCounts(4, 5, 6)));
        p->addResults(syncResults);

        // Save log.
        pm.saveLog(*p->log());
    }

    // Load profile. Log is loaded also.
    {
        QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
        QVERIFY(p != 0);
        const SyncLog *loadedLog = p->log();
        QVERIFY(loadedLog != 0);
        QCOMPARE(loadedLog->profileName(), OVI_CALENDAR);
        QVERIFY(p->lastResults() != 0);
        QCOMPARE(p->lastResults()->majorCode(), 1);
        QList<TargetResults> targetResults = p->lastResults()->targetResults();
        QCOMPARE(targetResults.size(), 1);
        QCOMPARE(targetResults[0].targetName(), QString("hcalendar"));
        QCOMPARE(targetResults[0].localItems().added, (unsigned)1);
        QCOMPARE(targetResults[0].localItems().deleted, (unsigned)2);
        QCOMPARE(targetResults[0].localItems().modified, (unsigned)3);
        QCOMPARE(targetResults[0].remoteItems().added, (unsigned)4);
        QCOMPARE(targetResults[0].remoteItems().deleted, (unsigned)5);
        QCOMPARE(targetResults[0].remoteItems().modified, (unsigned)6);
    }

    // Save results through ProfileManager.
    {
        QCOMPARE(QFile::remove(
                USERPROFILE_DIR + "/sync/logs/" + OVI_CALENDAR + ".log.xml"), true);
        SyncResults syncResults(QDateTime::currentDateTime(), Buteo::SyncResults::SYNC_RESULT_FAILED, Buteo::SyncResults::INTERNAL_ERROR);
        syncResults.setMajorCode(Buteo::SyncResults::SYNC_RESULT_SUCCESS);
        pm.saveSyncResults(OVI_CALENDAR, syncResults);
        QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
        QVERIFY(p != 0);
        const SyncLog *log = p->log();
        QVERIFY(log != 0);
        QVERIFY(log->lastResults() != 0);
        QVERIFY(log->lastResults()->majorCode() ==
            Buteo::SyncResults::SYNC_RESULT_SUCCESS);
    }
}

void ProfileManagerTest::testSave()
{
    ProfileManager pm(USERPROFILE_DIR + "/primary", USERPROFILE_DIR);

    {
        QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
        QVERIFY(p != 0);
        QCOMPARE(p->isEnabled(), true);
        p->setEnabled(false);
        // NOTE: removed addProfile()
//        pm.addProfile(*p);
    }

    {
        QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
        QVERIFY(p != 0);
        QCOMPARE(p->isEnabled(), false);

        // Profile file in secondary directory is not affected.
        {
            ProfileManager pm2(USERPROFILE_DIR, USERPROFILE_DIR);
            QScopedPointer<SyncProfile> p2(pm2.syncProfile(OVI_CALENDAR));
            QVERIFY(p2 != 0);
            QCOMPARE(p2->isEnabled(), true);
        }

        p->setEnabled(true);
        pm.updateProfile(*p);
    }
}

void ProfileManagerTest::testHiddenProfiles()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);

    // Get number of visible sync profiles.
    QList<SyncProfile*> profiles = pm.allVisibleSyncProfiles();
    int num_profiles = profiles.size();
    qDeleteAll(profiles);
    profiles.clear();

    // Make one of the profiles hidden.
    QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
    QVERIFY(p != 0);
    p->setBoolKey(KEY_HIDDEN, true);
//    pm.addProfile(*p);

    // Verify that number of visible profiles is reduced.
    profiles = pm.allVisibleSyncProfiles();
    QCOMPARE(profiles.size(), num_profiles - 1);
    qDeleteAll(profiles);
    profiles.clear();

    // Make profile visible again.
    p->removeKey(KEY_HIDDEN);
    pm.updateProfile(*p);
}

void ProfileManagerTest::testRemovingProfiles()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);

    // Create a temporary profile that can be deleted.
    const QString TEMP_NAME = "TempProfile";
    QScopedPointer<SyncProfile> p(pm.syncProfile(OVI_CALENDAR));
    QVERIFY(p != 0);
    p->setName(TEMP_NAME);
//    pm.addProfile(*p);

    // Try removing protected profile.
    p->setBoolKey(KEY_PROTECTED, true);
    pm.updateProfile(*p);
    QCOMPARE(pm.removeProfile(TEMP_NAME), false);

    // Disable protectiong and remove profile.
    p->removeKey(KEY_PROTECTED);
    pm.updateProfile(*p);
    QCOMPARE(pm.removeProfile(TEMP_NAME), true);
}

void ProfileManagerTest::testOverrideKey()
{
    ProfileManager pm(USERPROFILE_DIR, USERPROFILE_DIR);

    QScopedPointer<SyncProfile> p(static_cast<SyncProfile*>(
        pm.profile(OVI_CALENDAR, Profile::TYPE_SYNC)));
    QVERIFY(p != 0);
    Profile *service = p->subProfile("ovi.com", Profile::TYPE_SYNC);
    QVERIFY(service != 0);

    // Set username to the main profile (service section).
    const QString UNAME_KEY = "Username";
    const QString UNAME = "new user";
    service->setKey(UNAME_KEY, UNAME);

    // Service sub-profile file contains a user name also, but the key defined
    // in the main profile overrides it.
    pm.expand(*p);
    QCOMPARE(service->key(UNAME_KEY), UNAME);
}

void ProfileManagerTest::testBackup()
{
    ProfileManager pm(USERPROFILE_DIR + '/', SYSTEMPROFILE_DIR + '/'); 

    // Copy to backup.
    QString fileName = USERPROFILE_DIR + '/' + Profile::TYPE_SYNC +
        '/' + OVI_CALENDAR + ".xml";
    QFile file(fileName);
    QVERIFY(file.copy(fileName + ".bak"));

    // Get a sync profile. Profile is restored from a backup.
    QScopedPointer<SyncProfile> sp(pm.syncProfile(OVI_CALENDAR));
    QVERIFY(sp != 0);
    QCOMPARE(sp->name(), OVI_CALENDAR);
    QCOMPARE(sp->type(), Profile::TYPE_SYNC);

    // Backup is removed after successful load.
    QVERIFY(!QFile::exists(fileName + ".bak"));
}

TESTLOADER_ADD_TEST(ProfileManagerTest);
