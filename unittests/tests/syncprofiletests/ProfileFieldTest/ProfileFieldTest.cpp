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
#include "ProfileFieldTest.h"

#include <QDomDocument>

#include "ProfileField.h"

using namespace Buteo;

static const QString FIELD_XML =
    "<field name=\"Notebook Name\" type=\"combo\" default=\"myNotebook\" "
        "label=\"Notebook\" visible=\"user\" readonly=\"true\">"
        "<option>myNotebook</option>"
        "<option>otherNotebook</option>"
    "</field>";

void ProfileFieldTest::testField()
{
    QDomDocument doc;
    QVERIFY(doc.setContent(FIELD_XML, false));

    // Create from XML.
    ProfileField pf_original(doc.documentElement());
    // Copy construction.
    ProfileField pf(pf_original);

    // Verify properties.
    QCOMPARE(pf.name(), QString("Notebook Name"));
    QCOMPARE(pf.type(), QString("combo"));
    QCOMPARE(pf.defaultValue(), QString("myNotebook"));
    QCOMPARE(pf.label(), QString("Notebook"));
    QCOMPARE(pf.visible(), ProfileField::VISIBLE_USER);
    QCOMPARE(pf.isReadOnly(), true);
    QStringList options = pf.options();
    QCOMPARE(options.size(), 2);
    QCOMPARE(options[0], QString("myNotebook"));
    QCOMPARE(options[1], QString("otherNotebook"));

    // Test value validation.
    QCOMPARE(pf.validate("myNotebook"), true);
    QCOMPARE(pf.validate("otherNotebook"), true);
    QCOMPARE(pf.validate("something else"), false);

    // XML output.
    QDomDocument doc2;
    QDomElement root = pf.toXml(doc2);
    QVERIFY(!root.isNull());
    doc2.appendChild(root);
    QCOMPARE(doc.toString(), doc2.toString());
}

QTEST_MAIN(Buteo::ProfileFieldTest)
