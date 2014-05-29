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
#include "SyncLogTest.h"

#include <QDomDocument>

#include "SyncLog.h"

using namespace Buteo;

static const QString NAME = "ovi-calendar";
static const QString LOG_XML =
    "<synclog name=\"ovi-calendar\" >"
        "<syncresults majorcode=\"1\" time=\"2009-09-15T16:33:57\" scheduled=\"true\">"
            "<target name=\"hcalendar\">"
                "<local added=\"2\" deleted=\"3\" modified=\"4\" />"
                "<remote added=\"5\" deleted=\"6\" modified=\"7\" />"
            "</target>"
        "</syncresults>"
    "</synclog>";

void SyncLogTest::testLog()
{
    // Create from scratch.
    SyncLog log1(NAME);
    QCOMPARE(log1.profileName(), NAME);
    QVERIFY(log1.lastResults() == 0);
    QCOMPARE(log1.allResults().size(), 0);

    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(LOG_XML, false));
    SyncLog log2(doc.documentElement());
    QCOMPARE(log2.profileName(), NAME);
    QVERIFY(log2.lastResults() != 0);
    QCOMPARE(log2.allResults().size(), 1);
    TargetResults tr2 = log2.lastResults()->targetResults().at(0);
    QCOMPARE(tr2.targetName(), QString("hcalendar"));
    QCOMPARE(tr2.localItems().added, (unsigned)2);
    QCOMPARE(tr2.localItems().deleted, (unsigned)3);
    QCOMPARE(tr2.localItems().modified, (unsigned)4);
    QCOMPARE(tr2.remoteItems().added, (unsigned)5);
    QCOMPARE(tr2.remoteItems().deleted, (unsigned)6);
    QCOMPARE(tr2.remoteItems().modified, (unsigned)7);
    QCOMPARE(log2.lastResults()->isScheduled(), true);

    // Copy constructor.
    SyncLog log3(log2);
    QDomDocument doc2;
    doc2.appendChild(log2.toXml(doc2));
    QDomDocument doc3;
    doc3.appendChild(log3.toXml(doc3));
    QVERIFY(doc2.toString().size() >= LOG_XML.size());
    QCOMPARE(doc2.toString(), doc3.toString());

    // Add new results.
    SyncResults newResults;
    newResults.setMajorCode(Buteo::SyncResults::SYNC_RESULT_CANCELLED);
    QCOMPARE(newResults.majorCode(), 2);
    QCOMPARE(newResults < *log2.lastResults(), false);
    newResults.addTargetResults(TargetResults("hcontacts", ItemCounts(2, 3, 4),
        ItemCounts(5, 6, 7)));
    log2.addResults(newResults);
    QVERIFY(log2.lastResults() != 0);
    QCOMPARE(log2.lastResults()->majorCode(), 2);
    QCOMPARE(log2.allResults().size(), 2);
    QCOMPARE(log2.allResults().at(0)->majorCode(), 1);

    // Verify target results contents.
    QCOMPARE(log2.lastResults()->targetResults().size(), 1);
    TargetResults tr = log2.lastResults()->targetResults().at(0);
    QCOMPARE(tr.targetName(), QString("hcontacts"));
    QCOMPARE(tr.localItems().added, (unsigned)2);
    QCOMPARE(tr.localItems().deleted, (unsigned)3);
    QCOMPARE(tr.localItems().modified, (unsigned)4);
    QCOMPARE(tr.remoteItems().added, (unsigned)5);
    QCOMPARE(tr.remoteItems().deleted, (unsigned)6);
    QCOMPARE(tr.remoteItems().modified, (unsigned)7);

}

void SyncLogTest::testAddResults()
{
    const int MAXLOGENTRIES = 5;
    
    SyncLog log(NAME);
    SyncResults newResults;
    newResults.setMajorCode(Buteo::SyncResults::SYNC_RESULT_CANCELLED);
    newResults.addTargetResults(TargetResults("hcontacts", ItemCounts(2, 3, 4),
                                ItemCounts(5, 6, 7)));
    
    for (int i = 0; i < 7; ++i) {
        log.addResults(newResults);
        QVERIFY(log.allResults().size() <= MAXLOGENTRIES);
    }
    
    
}


QTEST_MAIN(Buteo::SyncLogTest)
