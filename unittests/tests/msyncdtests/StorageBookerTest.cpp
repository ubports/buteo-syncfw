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
#include "StorageBookerTest.h"
#include "StorageBooker.h"
#include "SyncFwTestLoader.h"

using namespace Buteo;

void StorageBookerTest::testBooking()
{
    const QString STORAGE1 = "Storage1";
    const QString STORAGE2 = "Storage2";
    const QString CLIENT1 = "Client1";
    const QString CLIENT2 = "Client2";
    const QString EMPTY_CLIENT = "";

    StorageBooker booker;
    QStringList allStorages;
    allStorages << STORAGE1 << STORAGE2;

    // No reservations yet, storages are available.
    QCOMPARE(booker.isStorageAvailable(STORAGE1, CLIENT1), true);
    QCOMPARE(booker.isStorageAvailable(STORAGE1, EMPTY_CLIENT), true);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT2), true);

    // Reserve storage1 for client1. Other clients are not able to reserve.
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT1), true);
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT2), false);
    QCOMPARE(booker.reserveStorage(STORAGE1, EMPTY_CLIENT), false);
    QCOMPARE(booker.isStorageAvailable(STORAGE1, CLIENT1), true);
    QCOMPARE(booker.isStorageAvailable(STORAGE1, CLIENT2), false);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT1), true);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT2), false);

    // Reserve more again, release gives correct remaingin ref count.
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT1), true);
    QCOMPARE(booker.releaseStorage(STORAGE1), (unsigned)1);
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT2), false);
    QCOMPARE(booker.releaseStorage(STORAGE1), (unsigned)0);

    // Another client is able to reserve after all references are released.
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT2), true);
    QCOMPARE(booker.releaseStorage(STORAGE1), (unsigned)0);

    // Reserve multiple storages at a time.
    QCOMPARE(booker.reserveStorages(allStorages, CLIENT2), true);
    QCOMPARE(booker.reserveStorages(allStorages, CLIENT1), false);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT2), true);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT1), false);
    booker.releaseStorages(allStorages);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT1), true);
    QCOMPARE(booker.reserveStorages(allStorages, EMPTY_CLIENT), true);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT1), false);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT2), false);
    QCOMPARE(booker.storagesAvailable(allStorages, EMPTY_CLIENT), false);
    QCOMPARE(booker.releaseStorage(STORAGE1), (unsigned)0);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT1), false);
    QCOMPARE(booker.releaseStorage(STORAGE2), (unsigned)0);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT1), true);

    // Reserve all, reserve individual, release individual, release all.
    QCOMPARE(booker.reserveStorages(allStorages, CLIENT1), true);
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT1), true);
    QCOMPARE(booker.reserveStorage(STORAGE2, CLIENT1), true);
    QCOMPARE(booker.reserveStorage(STORAGE1, CLIENT2), false);
    QCOMPARE(booker.reserveStorage(STORAGE2, CLIENT2), false);
    QCOMPARE(booker.releaseStorage(STORAGE1), (unsigned)1);
    QCOMPARE(booker.releaseStorage(STORAGE2), (unsigned)1);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT2), false);
    booker.releaseStorages(allStorages);
    QCOMPARE(booker.storagesAvailable(allStorages, CLIENT2), true);

}

TESTLOADER_ADD_TEST(StorageBookerTest);
