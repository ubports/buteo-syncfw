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
#include "StorageProfileTest.h"

#include <QDomDocument>

#include "StorageProfile.h"
#include "SyncFwTestLoader.h"

using namespace Buteo;

static const QString NAME = "hcalendar";
static const QString TYPE = Profile::TYPE_STORAGE;
static const QString PROFILE_XML =
    "<profile name=\"hcalendar\" type=\"storage\" >"
        "<field name=\"Notebook Name\" type=\"combo\" label=\"Notebook\">"
            "<option>myNotebook</option>"
            "<option>otherNotebook</option>"
        "</field>"
        "<key name=\"Local URI\" value=\"./Calendar\" />"
    "</profile>";

void StorageProfileTest::testStorageProfile()
{
    // Create from scratch.
    StorageProfile p1(NAME);
    QCOMPARE(p1.name(), NAME);
    QCOMPARE(p1.type(), TYPE);

    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(PROFILE_XML, false));
    StorageProfile p2(doc.documentElement());
    QCOMPARE(p2.name(), NAME);
    QCOMPARE(p2.type(), TYPE);

    // Copy constructor.
    StorageProfile p3(p2);
    QCOMPARE(p3.toString(), p2.toString());

    // Storage profile is disabled by default.
    Profile *p = &p1;
    QCOMPARE(p->isEnabled(), false);
    p->setEnabled(true);
    QCOMPARE(p->isEnabled(), true);
}

TESTLOADER_ADD_TEST(StorageProfileTest);
