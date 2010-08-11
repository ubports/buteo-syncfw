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
#include "SyncQueueTest.h"
#include "SyncQueue.h"
#include "SyncFwTestLoader.h"
#include "SyncSession.h"
#include <SyncProfile.h>

using namespace Buteo;

void SyncQueueTest::testQueue()
{
    const QString NAME1 = "Name1";
    const QString NAME2 = "Name2";
    SyncSession s1(new SyncProfile(NAME1));
    SyncSession s2(new SyncProfile(NAME2));
    SyncQueue q;

    // Empty queue.
    QCOMPARE(q.isEmpty(), true);
    QVERIFY(q.head() == NULL);
    QVERIFY(q.dequeue() == NULL);
    QCOMPARE(q.contains(NAME1), false);

    // Add items.
    q.enqueue(&s1);
    q.enqueue(&s2);
    QCOMPARE(q.isEmpty(), false);
    QCOMPARE(q.head(), &s1);
    QCOMPARE(q.contains(NAME1), true);
    QCOMPARE(q.contains(NAME2), true);
    QCOMPARE(q.dequeue(), &s1);
    QCOMPARE(q.contains(NAME1), false);
    QCOMPARE(q.contains(NAME2), true);
    QCOMPARE(q.isEmpty(), false);
    q.enqueue(&s1);
    QCOMPARE(q.head(), &s2);
    QCOMPARE(q.dequeue(), &s2);
    QCOMPARE(q.head(), &s1);
    QCOMPARE(q.dequeue(), &s1);
    QCOMPARE(q.isEmpty(), true);
    QVERIFY(q.dequeue() == NULL);

}

TESTLOADER_ADD_TEST(SyncQueueTest);
