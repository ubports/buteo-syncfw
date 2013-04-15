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
#include "ProfileTest.h"
#include "Profile_p.h"

#include <QDomDocument>
#include <QScopedPointer>

#include "SyncFwTestLoader.h"
#include "ProfileEngineDefs.h"
#include "Logger.h"

using namespace Buteo;

static const QString PROFILE_DIR = "syncprofiletests/testprofiles/user";

void ProfileTest::initTestCase()
{
    //Logger::createInstance();
}

void ProfileTest::cleanupTestCase()
{
    //Logger::deleteInstance();
}

void ProfileTest::testConstruction()
{
    QString name = "ovi-calendar";
    QString type = Profile::TYPE_SYNC;

    // Construction from scratch.
    {
        Profile p(name, type);

        QCOMPARE(p.name(), name);
        QCOMPARE(p.type(), type);
    }

    // Construction from XML.
    {
        QScopedPointer<Profile> p(loadFromXmlFile(name, Profile::TYPE_SYNC));

        QVERIFY(p != 0);
        QCOMPARE(p->name(), name);
        QCOMPARE(p->type(), type);
        QCOMPARE(p->isEnabled(), true);

        QStringList subProfileNames = p->subProfileNames();
        QCOMPARE(subProfileNames.size(), 3);

        Profile *sp = p->subProfile("hcalendar", Profile::TYPE_STORAGE);
        QVERIFY(sp != 0);
        QCOMPARE(sp->key("Notebook Name"), QString("myNotebook"));
    }

    // Copy constructor.
    {
        QScopedPointer<Profile> p(loadFromXmlFile("hcalendar", Profile::TYPE_STORAGE));
        QScopedPointer<Profile> p2(loadFromXmlFile("ovi.com", Profile::TYPE_SYNC));
        p2->merge(*p);
        Profile p3(*p2);

        QCOMPARE(p3.toString(), p2->toString());
    }
}

void ProfileTest::testProperties()
{
    QString name = "ovi-calendar";
    QString newName = "ovi-new";
    QString type = Profile::TYPE_SYNC;

    Profile p(name, type);

    // Change name.
    QCOMPARE(p.name(), name);
    p.setName(newName);
    QCOMPARE(p.name(), newName);

    // Change enabled status.
    QCOMPARE(p.isEnabled(), true);
    p.setEnabled(false);
    QCOMPARE(p.isEnabled(), false);

    // Change loaded status.
    QCOMPARE(p.isLoaded(), false);
    p.setLoaded(true);
    QCOMPARE(p.isLoaded(), true);
}

void ProfileTest::testKeys()
{
    const QString NAME = "ovi-calendar";
    const QString TYPE = Profile::TYPE_SYNC;
    const QString KEY1 = "key1";
    const QString VALUE1 = "value1";
    const QString KEY2 = "key2";
    const QString VALUE2 = "value2";
    const QString VALUE3 = "value3";
    const QString BOOLKEY = "boolkey";
    const QString DEFAULT = "default";

    Profile p(NAME, TYPE);

    // No keys.
    QVERIFY(p.key(KEY1).isNull());
    QCOMPARE(p.key(KEY1, DEFAULT), DEFAULT);
    QVERIFY(p.allKeys().isEmpty());
    QCOMPARE(p.boolKey(BOOLKEY), false); // Default = false
    QCOMPARE(p.boolKey(BOOLKEY, true), true); // Default = true
    QVERIFY(p.keyValues(KEY1).isEmpty());
    QVERIFY(p.keyNames().isEmpty());

    // Add keys.
    p.setKey(KEY1, VALUE1);
    QStringList values;
    values << VALUE2 << VALUE3;
    QCOMPARE(values.size(), 2);
    p.setKeyValues(KEY2, values);
    p.setBoolKey(BOOLKEY, true);
    QCOMPARE(p.key(KEY1), VALUE1);
    QCOMPARE(p.key(KEY2), VALUE2);
    QCOMPARE(p.boolKey(BOOLKEY), true);

    QMap<QString, QString> allKeys = p.allKeys();
    QCOMPARE(allKeys.size(), 4); // Note: two values for KEY2.
    QCOMPARE(allKeys.value(KEY1), VALUE1);
    QCOMPARE(allKeys.value(KEY2), VALUE2);
    QCOMPARE(allKeys.value(BOOLKEY), QString("true"));

    QStringList key2Values = p.keyValues(KEY2);
    QCOMPARE(key2Values.size(), 2);
    QCOMPARE(key2Values[0], VALUE2);
    QCOMPARE(key2Values[1], VALUE3);

    QStringList keyNames = p.keyNames();
    QCOMPARE(keyNames.size(), 3);
    QCOMPARE(keyNames[0], BOOLKEY);
    QCOMPARE(keyNames[1], KEY1);
    QCOMPARE(keyNames[2], KEY2);

    // Modify keys.
    p.setKey(KEY1, VALUE2);
    p.setKey(KEY2, VALUE1);
    p.setKey(BOOLKEY, false);
    QCOMPARE(p.key(KEY1), VALUE2);
    QCOMPARE(p.keyValues(KEY1).size(), 1);
    QCOMPARE(p.key(KEY2), VALUE1);
    QCOMPARE(p.boolKey(BOOLKEY), false);

    key2Values = p.keyValues(KEY2);
    QCOMPARE(key2Values.size(), 2);
    QCOMPARE(key2Values[0], VALUE1);
    QCOMPARE(key2Values[1], VALUE3);

    values.clear();
    p.setKeyValues(KEY2, values);
    QCOMPARE(p.keyValues(KEY2).size(), 0);

    values << VALUE2 << VALUE1;
    p.setKeyValues(KEY2, values);
    key2Values = p.keyValues(KEY2);
    QCOMPARE(key2Values.size(), 2);
    QCOMPARE(key2Values[0], VALUE2);
    QCOMPARE(key2Values[1], VALUE1);

    // Remove key.
    p.removeKey(KEY2);
    key2Values = p.keyValues(KEY2);
    QCOMPARE(key2Values.size(), 0);

}

void ProfileTest::testFields()
{
    // Load a profile that has fields: calendar storage profile.
    const QString NAME = "hcalendar";
    const QString TYPE = Profile::TYPE_STORAGE;
    QScopedPointer<Profile> p(loadFromXmlFile(NAME, TYPE));
    QVERIFY(p != 0);
    QCOMPARE(p->name(), NAME);
    QCOMPARE(p->type(), TYPE);

    // There should be 3 fields.
    QList<const ProfileField*> allFields = p->allFields();
    QCOMPARE(allFields.size(), 3);

    // The last field should be about notebook name.
    const ProfileField *field = p->field("Notebook Name");
    QCOMPARE(field, allFields.last());

    // Field that does not exist.
    QVERIFY(p->field("unknown") == 0);

    // Compare field properties.
    QCOMPARE(field->name(), QString("Notebook Name"));
    QCOMPARE(field->type(), QString("combo"));
    QCOMPARE(field->defaultValue(), QString("myNotebook"));
    QStringList options = field->options();
    QCOMPARE(options.size(), 2);
    QCOMPARE(options[0], QString("myNotebook"));
    QCOMPARE(options[1], QString("otherNotebook"));

    QCOMPARE(field->validate("myNotebook"), true);
    QCOMPARE(field->validate("otherNotebook"), true);
    QCOMPARE(field->validate("invalidNotebook"), false);

    // All fields are visible.
    QCOMPARE(p->visibleFields().size(), 3);
}

void ProfileTest::testSubProfiles()
{
    // Load a profile that has sub-profiles.
    const QString NAME = "ovi-calendar";
    const QString TYPE = Profile::TYPE_SYNC;
    QScopedPointer<Profile> p(loadFromXmlFile(NAME, TYPE));
    QVERIFY(p != 0);
    QCOMPARE(p->name(), NAME);
    QCOMPARE(p->type(), TYPE);

    QStringList subProfileNames = p->subProfileNames();
    QCOMPARE(subProfileNames[0], QString("ovi.com"));
    QCOMPARE(subProfileNames[1], QString("hcalendar"));
    QCOMPARE(subProfileNames[2], QString("hcontacts"));

    QCOMPARE(p->subProfileNames(Profile::TYPE_CLIENT).size(), 0);
    QCOMPARE(p->subProfileNames(Profile::TYPE_SYNC).size(), 1);
    QCOMPARE(p->subProfileNames(Profile::TYPE_STORAGE).size(), 2);

    // Sub-profile that does not exist.
    QVERIFY(p->subProfile("unknown", Profile::TYPE_CLIENT) == 0);

    const Profile *const_p = p.data();
    const Profile *sub = const_p->subProfile("hcalendar", Profile::TYPE_STORAGE);
    QVERIFY(sub != 0);
    QCOMPARE(sub, p->subProfile("hcalendar"));

    QCOMPARE(sub->isEnabled(), true);
    QCOMPARE(sub->key("Notebook Name"), QString("myNotebook"));

    QList<Profile*> allSubProfiles = p->allSubProfiles();
    QCOMPARE(allSubProfiles.size(), 3);

    // Sub-profile by key value.
    sub = p->subProfileByKeyValue("Notebook Name", "myNotebook",
        Profile::TYPE_STORAGE, true);
    QVERIFY(sub != 0);
    QCOMPARE(sub, p->subProfile("hcalendar"));
}

void ProfileTest::testMerge()
{
    QScopedPointer<Profile> p(loadFromXmlFile("ovi.com", Profile::TYPE_SYNC));
    QScopedPointer<Profile> p2(loadFromXmlFile("hcalendar", Profile::TYPE_STORAGE));

    QVERIFY(p != 0);
    QVERIFY(p2 != 0);

    Profile *sub = p->subProfile("hcalendar");
    QVERIFY(sub != 0);
    QVERIFY(sub->key("Local URI").isNull());
    QVERIFY(sub->allFields().isEmpty());
    p->merge(*p2);
    QCOMPARE(sub->key("Local URI"), QString("./Calendar"));
    QCOMPARE(sub->allFields().size(), 3);

    QCOMPARE(sub->d_ptr->iLocalKeys.size(), 2);
    QCOMPARE(sub->d_ptr->iMergedKeys.size(), 1);
    QCOMPARE(sub->d_ptr->iLocalFields.size(), 0);
    QCOMPARE(sub->d_ptr->iMergedFields.size(), 3);

    // Merge service to sync profile.
    QScopedPointer<Profile> p3(loadFromXmlFile("ovi-calendar", Profile::TYPE_SYNC));
    QVERIFY(p3 != 0);
    p3->merge(*p);
    QVERIFY(p3->subProfile("syncml", Profile::TYPE_CLIENT) != 0);
}

void ProfileTest::testValidate()
{
    QScopedPointer<Profile> p(loadFromXmlFile("hcalendar", Profile::TYPE_STORAGE));
    QVERIFY(p != 0);

    // Invalid, because field values are not set.
    QCOMPARE(p->isValid(), false);

    // Valid after setting field values.
    p->setKey("Target URI", "cal");
    p->setKey("Calendar Format", "vcalendar");
    p->setKey("Notebook Name", "myNotebook");
    QCOMPARE(p->isValid(), true);

    // Invalid when name or type is empty.
    p->setName("");
    QCOMPARE(p->isValid(), false);
    p->setName("hcalendar");
    p->d_ptr->iType = "";
    QCOMPARE(p->isValid(), false);
    p->d_ptr->iType = Profile::TYPE_STORAGE;
    QCOMPARE(p->isValid(), true);

    // Invalid when field value is invalid.
    p->setKey("Notebook Name", "invalid");
    QCOMPARE(p->isValid(), false);
    p->setKey("Notebook Name", "myNotebook");

    // Validate profile that has sub-profiles.
    QScopedPointer<Profile> p2(loadFromXmlFile("ovi.com", Profile::TYPE_SYNC));
    QVERIFY(p2 != 0);
    p->setEnabled(true);
    p2->merge(*p);
    QCOMPARE(p2->isValid(), true);

    // Removing required key makes the profile invalid.
    Profile *sub = p2->subProfile("hcalendar");
    QVERIFY(sub != 0);
    sub->setKey("Target URI", QString::null);
    QCOMPARE(p2->isValid(), false);
    // Disabling the profile that requires the removed key makes the profile
    // valid again.
    sub->setEnabled(false);
    QCOMPARE(p2->isValid(), true);
}

void ProfileTest::testXmlConversion()
{
    QScopedPointer<Profile> p(loadFromXmlFile("hcalendar", Profile::TYPE_STORAGE));
    QVERIFY(p != 0);
    QVERIFY(saveToXmlFile(*p, "hcalendar-output"));
    QCOMPARE(profileFileToString("hcalendar-output", Profile::TYPE_STORAGE),
        profileFileToString("hcalendar-expected", Profile::TYPE_STORAGE));

    // Merge storage profile to service profile.
    QScopedPointer<Profile> p2(loadFromXmlFile("ovi.com", Profile::TYPE_SYNC));
    QVERIFY(p2 != 0);
    p2->merge(*p);

    // Output local profile data only, no merged sub-profile data.
    QVERIFY(saveToXmlFile(*p2, "ovi.com-output"));
    QCOMPARE(profileFileToString("ovi.com-output", Profile::TYPE_SYNC),
        profileFileToString("ovi.com-expected", Profile::TYPE_SYNC));

    // Output merged sub-profiles also.
    QVERIFY(saveToXmlFile(*p2, "ovi.com-output", false));
    QCOMPARE(profileFileToString("ovi.com-output", Profile::TYPE_SYNC),
        profileFileToString("ovi.com-merged-expected", Profile::TYPE_SYNC));
}

Profile *ProfileTest::loadFromXmlFile(const QString &aName, const QString &aType)
{
    QFile file(PROFILE_DIR + "/" + aType + "/" + aName + ".xml");

    if (!file.open(QIODevice::ReadOnly))
    {
        return 0;
    } // no else

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return 0;
    } // no else
    file.close();

    return new Profile(doc.documentElement());
}

bool ProfileTest::saveToXmlFile(const Profile &aProfile, const QString &aName,
                                bool aLocalOnly)
{
    QDir dir;
    dir.mkpath(PROFILE_DIR + "/" + aProfile.type());
    QFile file(PROFILE_DIR + "/" + aProfile.type() + "/" +
        aName + ".xml");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    } // no else

    QDomDocument doc;
    QDomProcessingInstruction xmlHeading =
        doc.createProcessingInstruction("xml",
        "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlHeading);

    QDomElement root = aProfile.toXml(doc, aLocalOnly);
    if (root.isNull())
    {
        return false;
    } // no else

    doc.appendChild(root);

    QTextStream outputStream(&file);

    outputStream << doc.toString(PROFILE_INDENT);

    file.close();

    return true;
}

QString ProfileTest::profileFileToString(const QString &aName,
                                         const QString &aType)
{
    QString output;

    QFile file(PROFILE_DIR + "/" + aType + "/" + aName + ".xml");

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream outputStream(&file);
        output = outputStream.readAll();
    } // no else

    return output;
}


TESTLOADER_ADD_TEST(ProfileTest);
