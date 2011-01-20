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
#include "SyncScheduleTest.h"
#include "SyncSchedule.h"
#include "SyncSchedule_p.h"
#include "SyncFwTestLoader.h"

#include <QDomDocument>

using namespace Buteo;

static const QString SCHEDULE_XML =
    "<schedule time=\"12:34:56\" interval=\"30\" days=\"1,2,3,4,5,6\">"
        "<rush enabled=\"true\" interval=\"15\""
            "begin=\"08:00:00\" end=\"16:00:00\" days=\"1,4,5\"/>"
    "</schedule>";

void SyncScheduleTest::testConstruction()
{
    // Create from scratch.
    SyncSchedule s;
    QCOMPARE(s.interval(), (unsigned)0);
    QCOMPARE(s.time().isNull(), true);
    QCOMPARE(s.days().isEmpty(), true);
    QCOMPARE(s.rushEnabled(), false);
    QCOMPARE(s.rushInterval(), (unsigned)0);
    QCOMPARE(s.rushDays().isEmpty(), true);

    // Create from XML.
    QDomDocument doc;
    QVERIFY(doc.setContent(SCHEDULE_XML, false));
    SyncSchedule s2(doc.documentElement());
    QCOMPARE(s2.interval(), (unsigned)30);
    QVERIFY(s2.time() == QTime(12, 34, 56, 0));
    DaySet days = s2.days();
    QCOMPARE(days.contains(Qt::Monday), true);
    QCOMPARE(days.contains(Qt::Tuesday), true);
    QCOMPARE(days.contains(Qt::Wednesday), true);
    QCOMPARE(days.contains(Qt::Thursday), true);
    QCOMPARE(days.contains(Qt::Friday), true);
    QCOMPARE(days.contains(Qt::Saturday), true);
    QCOMPARE(days.contains(Qt::Sunday), false);
    QCOMPARE(s2.rushEnabled(), true);
    QCOMPARE(s2.rushInterval(), (unsigned)15);
    QVERIFY(s2.rushBegin() == QTime(8, 0, 0, 0));
    QVERIFY(s2.rushEnd() == QTime(16, 0, 0, 0));
    DaySet rushDays = s2.rushDays();
    QCOMPARE(rushDays.contains(Qt::Monday), true);
    QCOMPARE(rushDays.contains(Qt::Tuesday), false);
    QCOMPARE(rushDays.contains(Qt::Wednesday), false);
    QCOMPARE(rushDays.contains(Qt::Thursday), true);
    QCOMPARE(rushDays.contains(Qt::Friday), true);
    QCOMPARE(rushDays.contains(Qt::Saturday), false);
    QCOMPARE(rushDays.contains(Qt::Sunday), false);

    // Copy constructor.
    SyncSchedule s3(s2);
    QDomDocument doc2;
    QDomDocument doc3;
    doc2.appendChild(s2.toXml(doc2));
    doc3.appendChild(s3.toXml(doc3));
    QVERIFY(doc2.toString().size() >= SCHEDULE_XML.size());
    QCOMPARE(doc2.toString(), doc3.toString());
}

void SyncScheduleTest::testProperties()
{
    SyncSchedule s;
    DaySet d;
    d.insert(Qt::Tuesday);
    d.insert(Qt::Saturday);
    s.setDays(d);
    QVERIFY(s.days() == d);
    QTime t(1, 2, 3, 0);
    s.setTime(t);
    QVERIFY(s.time() == t);
    unsigned interval = 20;
    s.setInterval(interval);
    QVERIFY(s.interval() == interval);
    s.setRushEnabled(true);
    QVERIFY(s.rushEnabled() == true);
    d.insert(Qt::Wednesday);
    s.setRushDays(d);
    QVERIFY(s.rushDays() == d);
    QTime rushBegin(8, 0, 0, 0);
    QTime rushEnd(16, 0, 0, 0);
    s.setRushTime(rushBegin, rushEnd);
    QVERIFY(s.rushBegin() == rushBegin);
    QVERIFY(s.rushEnd() == rushEnd);
    unsigned rushInterval = 5;
    s.setRushInterval(rushInterval);
    QVERIFY(s.rushInterval() == rushInterval);
}


void SyncScheduleTest::testNextSyncTime()
{
    const unsigned INTERVAL = 30;
    const unsigned RUSH_INTERVAL = 10;
    SyncSchedule s;
    QDateTime previous(QDate(2009, 10, 7), QTime(11, 0, 0, 0));
    QDateTime now(QDate(2009, 10, 7), QTime(12, 0, 0, 0));

    // No schedule settings.
    QVERIFY(s.nextSyncTime(previous).isNull());

    // Exact time.
    QTime exact(15, 0, 0, 0);
    s.setTime(exact);
    QDateTime next = s.nextSyncTime(previous);
    QVERIFY(next.isNull());
    DaySet days;
    days.insert(Qt::Wednesday);
    days.insert(Qt::Monday);
    s.setDays(days);
    next = s.nextSyncTime(previous);
    QVERIFY(!next.isNull());
    QVERIFY(next.date() == now.date());
    QVERIFY(next.time() == exact);
    now.setTime(QTime(15, 0, 1, 0));
    next = s.nextSyncTime(previous);
    QVERIFY(next.date() == now.date().addDays(5));
    QVERIFY(next.time() == exact);

    // Interval.
    s.setTime(QTime());
    s.setInterval(INTERVAL);
    next = s.nextSyncTime(previous);
    QVERIFY(next == previous.addSecs(INTERVAL * 60));

    // Interval, no previous sync.
    next = s.nextSyncTime(QDateTime());
    QVERIFY(next == now);

    // Interval, sync missed.
    next = s.nextSyncTime(previous); // now = previous + 1h
    QVERIFY(next == now);

    // Interval, across day boundary, disabled days in the middle.
    previous.setTime(QTime(23, 50, 0, 0));
    next = s.nextSyncTime(previous);
    QVERIFY(next.date() == previous.addDays(5).date()); // Disabled days are skipped.
    QVERIFY(next.time() == QTime(0, 0, 0, 0)); // Sync as soon as day starts.

    // Rush enabled, no effect.
    DaySet rushDays;
    rushDays.insert(Qt::Monday);
    rushDays.insert(Qt::Friday);
    s.setRushDays(rushDays);
    s.setRushEnabled(true);
    s.setRushTime(QTime(8, 0, 0, 0), QTime(16, 0, 0, 0));
    s.setRushInterval(RUSH_INTERVAL);
    previous.setTime(QTime(11, 50, 0, 0));
    next = s.nextSyncTime(previous);
    QVERIFY(next == previous.addSecs(INTERVAL * 60));

    // No previous sync.
    s.setInterval(0);
    next = s.nextSyncTime(QDateTime());
    QVERIFY(next == now);
    s.setInterval(INTERVAL);

    // Currently in rush, sync missed.
    now.setDate(QDate(2009, 10, 9));
    next = s.nextSyncTime(previous);
    QVERIFY(next == now);

    // Currently in rush, previous in rush.
    now.setTime(QTime(12, 0, 0, 0));
    QVERIFY(s.d_ptr->isRush(now));
    next = s.nextSyncTime(now);
    QCOMPARE(next, now.addSecs(RUSH_INTERVAL * 60));

    // Currently in rush, next out of rush.
    now.setTime(QTime(15, 55, 0, 0));
    next = s.nextSyncTime(now);
    QCOMPARE(next.date().dayOfWeek(), (int)Qt::Monday);
    QCOMPARE(next.time(), QTime(0, 0, 0, 0));
    s.setInterval(0);
    next = s.nextSyncTime(now);
    QCOMPARE(next.date().dayOfWeek(), (int)Qt::Monday);
    QCOMPARE(next.time(), s.rushBegin());

    // Not currently in rush.
    now.setTime(QTime(6, 0, 0, 0));
    next = s.nextSyncTime(now);
    QCOMPARE(next.date(), now.date());
    QCOMPARE(next.time(), s.rushBegin());
    now.setTime(QTime(17, 0, 0, 0));
    next = s.nextSyncTime(now);
    QCOMPARE(next.date().dayOfWeek(), (int)Qt::Monday);
    QCOMPARE(next.time(), s.rushBegin());
}

TESTLOADER_ADD_TEST(SyncScheduleTest);
