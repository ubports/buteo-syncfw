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
#include "DeletedItemsIdStorageTest.h"
#include "DeletedItemsIdStorage.h"

#include <QList>
#include <QDateTime>

using namespace Buteo;

const QString DBFILE("/tmp/deleteditemsidstoragetest.db");


/*!
    \fn DeletedItemsIdStorageTest::init()
 */
void DeletedItemsIdStorageTest::init()
{
    iDeletedItems = new DeletedItemsIdStorage;
}


/*!
    \fn DeletedItemsIdStorageTest::cleanup()
 */
void DeletedItemsIdStorageTest::cleanup()
{
    delete iDeletedItems;
    iDeletedItems = NULL;
}

/*!
    \fn DeletedItemsIdStorageTest::testInit()
 */
void DeletedItemsIdStorageTest::testInit()
{
    bool initResult;
    bool uninitResult;

    initResult = iDeletedItems->init(DBFILE);
    QVERIFY(initResult == true);
    uninitResult = iDeletedItems->uninit();
    QVERIFY(uninitResult == true);


}


/*!
    \fn DeletedItemsIdStorageTest::testItemIdStoring()
 */
void DeletedItemsIdStorageTest::testItemIdStoring()
{
    iDeletedItems->init(DBFILE);

    QDateTime creationTime = QDateTime::fromTime_t(100000);
    QDateTime deletionTime = QDateTime::fromTime_t(20000000);

    QString itemId = "foo1";

    iDeletedItems->addDeletedItem(itemId, creationTime, deletionTime);

    QDateTime fetchTime = QDateTime::fromTime_t(5000000);
    QList<QString> itemIdList;
    bool success = iDeletedItems->getDeletedItems(itemIdList, fetchTime);

    QVERIFY(success);
    QVERIFY(itemIdList.size() > 0);
    QVERIFY(itemIdList.first() == itemId);

    iDeletedItems->uninit();

}


/*!
    \fn DeletedItemsIdStorageTest::testSnapshot()
 */
void DeletedItemsIdStorageTest::testSnapshot()
{

    iDeletedItems->init(DBFILE);

    QDateTime now = QDateTime::currentDateTime();
    // Zero milliseconds.
    now.setTime(now.time().addMSecs(-now.time().msec()));

    QList<QString> setItemIdList;
    setItemIdList << "foo" << "bar" << "zed";
    QList<QDateTime> setCreationTimes;
    setCreationTimes << now << now << now;
    iDeletedItems->setSnapshot(setItemIdList, setCreationTimes);

    QList<QString> receivedItemIdList;
    QList<QDateTime> receivedCreationTimes;
    iDeletedItems->getSnapshot(receivedItemIdList, receivedCreationTimes);

    QVERIFY(setItemIdList == receivedItemIdList);
    QVERIFY(setCreationTimes == receivedCreationTimes);

    iDeletedItems->uninit();
}


QTEST_MAIN(Buteo::DeletedItemsIdStorageTest)
