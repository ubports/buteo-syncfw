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
#include "ProfileFactoryTest.h"

#include <QDomDocument>
#include <QScopedPointer>

#include "ProfileFactory.h"
#include "SyncProfile.h"
#include "StorageProfile.h"

using namespace Buteo;

void ProfileFactoryTest::testCreateDirect()
{
    const QString NAME = "name";
    ProfileFactory pf;

    {
        QString type = Profile::TYPE_CLIENT;
        QScopedPointer<Profile> p(pf.createProfile(NAME, type));
        QVERIFY(p != 0);
        QCOMPARE(p->name(), NAME);
        QCOMPARE(p->type(), type);
    }

    {
        QString type = Profile::TYPE_SYNC;
        QScopedPointer<Profile> p(pf.createProfile(NAME, type));
        QVERIFY(p != 0);
        QCOMPARE(p->name(), NAME);
        QCOMPARE(p->type(), type);
        QVERIFY(dynamic_cast<SyncProfile *>(p.data()) != 0);
    }

    {
        QString type = Profile::TYPE_STORAGE;
        QScopedPointer<Profile> p(pf.createProfile(NAME, type));
        QVERIFY(p != 0);
        QCOMPARE(p->name(), NAME);
        QCOMPARE(p->type(), type);
        QVERIFY(dynamic_cast<StorageProfile *>(p.data()) != 0);
    }
}

void ProfileFactoryTest::testCreateFromXml()
{
    const QString NAME = "name";
    ProfileFactory pf;
    QDomDocument doc;

    {
        QString type = Profile::TYPE_CLIENT;
        Profile clientProfile(NAME, type);
        QDomElement root = clientProfile.toXml(doc);
        QScopedPointer<Profile> p(pf.createProfile(root));
        QVERIFY(p != 0);
        QCOMPARE(p->name(), NAME);
        QCOMPARE(p->type(), type);
    }

    {
        QString type = Profile::TYPE_SYNC;
        SyncProfile syncProfile(NAME);
        QDomElement root = syncProfile.toXml(doc);
        QScopedPointer<Profile> p(pf.createProfile(root));
        QVERIFY(p != 0);
        QCOMPARE(p->name(), NAME);
        QCOMPARE(p->type(), type);
        QVERIFY(dynamic_cast<SyncProfile *>(p.data()) != 0);
    }

    {
        QString type = Profile::TYPE_STORAGE;
        StorageProfile storageProfile(NAME);
        QDomElement root = storageProfile.toXml(doc);
        QScopedPointer<Profile> p(pf.createProfile(root));
        QVERIFY(p != 0);
        QCOMPARE(p->name(), NAME);
        QCOMPARE(p->type(), type);
        QVERIFY(dynamic_cast<StorageProfile *>(p.data()) != 0);
    }

}

QTEST_GUILESS_MAIN(Buteo::ProfileFactoryTest)
