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
    QVERIFY(log1.lastSuccessfulResults() == 0);

    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(LOG_XML, false));
    SyncLog log2(doc.documentElement());
    QCOMPARE(log2.profileName(), NAME);
    QVERIFY(log2.lastResults() != 0);
    QVERIFY(log2.lastSuccessfulResults() == 0);
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

    // Add a successful results.
    QDateTime successTime = QDateTime::fromString("2017-08-30T11:53:27",
                                                  "yyyy-MM-ddThh:mm:ss");
    log2.addResults(SyncResults(successTime, Buteo::SyncResults::SYNC_RESULT_SUCCESS,
                                Buteo::SyncResults::NO_ERROR));
    QVERIFY(log2.lastSuccessfulResults() != 0);
    QCOMPARE(log2.lastSuccessfulResults()->syncTime(), successTime);

    // Add new results.
    SyncResults newResults;
    newResults.setMajorCode(Buteo::SyncResults::SYNC_RESULT_CANCELLED);
    QCOMPARE(newResults.majorCode(), SyncResults::SYNC_RESULT_CANCELLED);
    QCOMPARE(newResults < *log2.lastResults(), false);
    newResults.addTargetResults(TargetResults("hcontacts", ItemCounts(2, 3, 4),
                                              ItemCounts(5, 6, 7)));
    log2.addResults(newResults);
    QVERIFY(log2.lastResults() != 0);
    QCOMPARE(log2.lastResults()->majorCode(), SyncResults::SYNC_RESULT_CANCELLED);
    QCOMPARE(log2.allResults().size(), 3);
    QCOMPARE(log2.allResults().at(0)->majorCode(), SyncResults::SYNC_RESULT_FAILED);
    QVERIFY(log2.lastSuccessfulResults() != 0);
    QCOMPARE(log2.lastSuccessfulResults()->syncTime(), successTime);

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

    // Update last successful results.
    successTime = QDateTime::fromString("2017-08-30T12:00:00", "yyyy-MM-ddThh:mm:ss");
    log2.addResults(SyncResults(successTime, Buteo::SyncResults::SYNC_RESULT_SUCCESS,
                                Buteo::SyncResults::NO_ERROR));
    QVERIFY(log2.lastSuccessfulResults() != 0);
    QCOMPARE(log2.lastSuccessfulResults()->syncTime(), successTime);

    // Push out last successful results from allResults.
    SyncResults failed;
    failed.setMajorCode(Buteo::SyncResults::SYNC_RESULT_FAILED);
    log2.addResults(failed);
    log2.addResults(failed);
    log2.addResults(failed);
    log2.addResults(failed);
    log2.addResults(failed);
    log2.addResults(failed);
    QCOMPARE(log2.allResults().size(), 5);
    QVERIFY(log2.lastSuccessfulResults() != 0);
    QCOMPARE(log2.lastSuccessfulResults()->syncTime(), successTime);
    QDomDocument docFailed;
    docFailed.appendChild(log2.toXml(docFailed));
    SyncLog logFailed(docFailed.documentElement());
    QCOMPARE(logFailed.allResults().size(), 5);
    QVERIFY(logFailed.lastSuccessfulResults() != 0);
    QCOMPARE(logFailed.lastSuccessfulResults()->syncTime(), successTime);
    QDomDocument docFailed2;
    docFailed2.appendChild(logFailed.toXml(docFailed2));
    QCOMPARE(docFailed.toString(), docFailed2.toString());
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

#define FAILURE_MESSAGE "Database error: UID not unique"
#define FAILURE_SERVER "No resource at URI"
static const QString DETAILS_XML =
    "<synclog name=\"caldav-sync\" >"
    "<syncresults majorcode=\"1\" time=\"2020-08-13T12:06:57\">"
    "<target name=\"123456-789\">"
    "<local added=\"2\" deleted=\"3\" modified=\"4\">"
    "<addedItem uid=\"123-4\" />"
    "<addedItem uid=\"123-5\" />"
    "<addedItem uid=\"123-6\" status=\"FaIleD\"><![CDATA["
    FAILURE_MESSAGE
    "]]></addedItem>"
    "<deletedItem uid=\"123-7\" />"
    "<modifiedItem uid=\"123-8\" />"
    "<modifiedItem uid=\"123-9\" />"
    "</local>"
    "<remote added=\"5\" deleted=\"6\" modified=\"7\">"
    "<addedItem uid=\"456-1\" />"
    "<deletedItem uid=\"456-2\" />"
    "<modifiedItem uid=\"456-3\" />"
    "<modifiedItem uid=\"456-7\" status=\"failed\"><![CDATA["
    FAILURE_SERVER
    "]]></modifiedItem>"
    "</remote>"
    "</target>"
    "</syncresults>"
    "</synclog>";
void SyncLogTest::testDetailsFromXML()
{
    QDomDocument doc;
    QVERIFY(doc.setContent(DETAILS_XML, false));
    SyncLog log(doc.documentElement());
    QCOMPARE(log.allResults().length(), 1);

    const SyncResults *result = log.lastResults();
    QVERIFY(result);
    QCOMPARE(result->targetResults().length(), 1);

    TargetResults target = result->targetResults().first();
    QCOMPARE(target.localDetails(TargetResults::ITEM_ADDED,
                                 TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("123-4") << QLatin1String("123-5"));
    QCOMPARE(target.localDetails(TargetResults::ITEM_ADDED,
                                 TargetResults::ITEM_OPERATION_FAILED),
             QList<QString>() << QLatin1String("123-6"));
    QCOMPARE(target.localMessage(QLatin1String("123-6")),
             QLatin1String(FAILURE_MESSAGE));
    QCOMPARE(target.localDetails(TargetResults::ITEM_DELETED,
                                 TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("123-7"));
    QVERIFY(target.localDetails(TargetResults::ITEM_DELETED,
                                TargetResults::ITEM_OPERATION_FAILED).isEmpty());
    QCOMPARE(target.localDetails(TargetResults::ITEM_MODIFIED,
                                 TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("123-8") << QLatin1String("123-9"));
    QVERIFY(target.localDetails(TargetResults::ITEM_MODIFIED,
                                TargetResults::ITEM_OPERATION_FAILED).isEmpty());

    QCOMPARE(target.remoteDetails(TargetResults::ITEM_ADDED,
                                  TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("456-1"));
    QVERIFY(target.remoteDetails(TargetResults::ITEM_ADDED,
                                 TargetResults::ITEM_OPERATION_FAILED).isEmpty());
    QCOMPARE(target.remoteDetails(TargetResults::ITEM_DELETED,
                                  TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("456-2"));
    QVERIFY(target.remoteDetails(TargetResults::ITEM_DELETED,
                                 TargetResults::ITEM_OPERATION_FAILED).isEmpty());
    QCOMPARE(target.remoteDetails(TargetResults::ITEM_MODIFIED,
                                  TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("456-3"));
    QCOMPARE(target.remoteDetails(TargetResults::ITEM_MODIFIED,
                                  TargetResults::ITEM_OPERATION_FAILED),
             QList<QString>() << QLatin1String("456-7"));
    QCOMPARE(target.remoteMessage(QLatin1String("456-7")),
             QLatin1String(FAILURE_SERVER));
}

void SyncLogTest::testAddDetails()
{
    TargetResults result(QLatin1String("Test target"));

    result.addLocalDetails(QLatin1String("123456-7"), TargetResults::ITEM_ADDED);
    result.addLocalDetails(QLatin1String("123456-8"), TargetResults::ITEM_ADDED);
    result.addLocalDetails(QLatin1String("123456-9"), TargetResults::ITEM_MODIFIED);
    result.addLocalDetails(QLatin1String("123456-10"), TargetResults::ITEM_DELETED);
    result.addLocalDetails(QLatin1String("123456-11"), TargetResults::ITEM_ADDED,
                           TargetResults::ITEM_OPERATION_FAILED,
                           QLatin1String(FAILURE_MESSAGE));
    QCOMPARE(result.localItems().added, (unsigned)2);
    QCOMPARE(result.localItems().deleted, (unsigned)1);
    QCOMPARE(result.localItems().modified, (unsigned)1);
    QCOMPARE(result.localDetails(TargetResults::ITEM_ADDED,
                                 TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("123456-7") << QLatin1String("123456-8"));
    QCOMPARE(result.localDetails(TargetResults::ITEM_ADDED,
                                 TargetResults::ITEM_OPERATION_FAILED),
             QList<QString>() << QLatin1String("123456-11"));
    QCOMPARE(result.localMessage(QLatin1String("123456-11")),
             QLatin1String(FAILURE_MESSAGE));
    QCOMPARE(result.localDetails(TargetResults::ITEM_DELETED,
                                 TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("123456-10"));
    QVERIFY(result.localDetails(TargetResults::ITEM_DELETED,
                                TargetResults::ITEM_OPERATION_FAILED).isEmpty());
    QCOMPARE(result.localDetails(TargetResults::ITEM_MODIFIED,
                                 TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("123456-9"));
    QVERIFY(result.localDetails(TargetResults::ITEM_MODIFIED,
                                TargetResults::ITEM_OPERATION_FAILED).isEmpty());

    result.addRemoteDetails(QLatin1String("147258-7"), TargetResults::ITEM_ADDED);
    result.addRemoteDetails(QLatin1String("147258-8"), TargetResults::ITEM_ADDED);
    result.addRemoteDetails(QLatin1String("147258-9"), TargetResults::ITEM_MODIFIED);
    result.addRemoteDetails(QLatin1String("147258-10"), TargetResults::ITEM_DELETED);
    result.addRemoteDetails(QLatin1String("147258-11"), TargetResults::ITEM_ADDED,
                            TargetResults::ITEM_OPERATION_FAILED,
                            QLatin1String(FAILURE_SERVER));
    QCOMPARE(result.remoteItems().added, (unsigned)2);
    QCOMPARE(result.remoteItems().deleted, (unsigned)1);
    QCOMPARE(result.remoteItems().modified, (unsigned)1);
    QCOMPARE(result.remoteDetails(TargetResults::ITEM_ADDED,
                                  TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("147258-7") << QLatin1String("147258-8"));
    QCOMPARE(result.remoteDetails(TargetResults::ITEM_ADDED,
                                  TargetResults::ITEM_OPERATION_FAILED),
             QList<QString>() << QLatin1String("147258-11"));
    QCOMPARE(result.remoteMessage(QLatin1String("147258-11")),
             QLatin1String(FAILURE_SERVER));
    QCOMPARE(result.remoteDetails(TargetResults::ITEM_DELETED,
                                  TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("147258-10"));
    QVERIFY(result.remoteDetails(TargetResults::ITEM_DELETED,
                                 TargetResults::ITEM_OPERATION_FAILED).isEmpty());
    QCOMPARE(result.remoteDetails(TargetResults::ITEM_MODIFIED,
                                  TargetResults::ITEM_OPERATION_SUCCEEDED),
             QList<QString>() << QLatin1String("147258-9"));
    QVERIFY(result.remoteDetails(TargetResults::ITEM_MODIFIED,
                                 TargetResults::ITEM_OPERATION_FAILED).isEmpty());
}

QTEST_GUILESS_MAIN(Buteo::SyncLogTest)
