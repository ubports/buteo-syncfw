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


#include "TransportTrackerTest.h"
#include <SyncCommonDefs.h>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QDBusVariant>
#include "SyncFwTestLoader.h"

using namespace Buteo;

void TransportTrackerTest :: initTestCase()
{
    // instantiating TransportTrackerTest
    iTransportTracker = new TransportTracker(this);

    /* TODO : For the following test cases, conditional compilation flag need to be added, as these
     * fail under scractch box */

    /*
    // check memory allocation for HalProxy
    QVERIFY(iTransportTracker->iHalProxy);
    // check memory allocation for USB
    // check for valid pointer of iInternet
    QVERIFY(iTransportTracker->iInternet);
    */
}

void TransportTrackerTest :: cleanupTestCase()
{
    // deallocate the memory
    delete iTransportTracker;

}

void TransportTrackerTest :: testConnectivityAvailable()
{
    // set the connectivity status for each type and compare it with the value returned

    // first set value as false
    iTransportTracker->updateState(Sync::CONNECTIVITY_USB, false);
    iTransportTracker->updateState(Sync::CONNECTIVITY_BT, false);
    iTransportTracker->updateState(Sync::CONNECTIVITY_INTERNET, false);

    bool usbTransportStatus = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_USB);
    QCOMPARE(usbTransportStatus, false);
    bool btTransportStatus = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_BT);
    QCOMPARE(btTransportStatus, false);
    bool internetTransportStatus = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_INTERNET);
    QCOMPARE(internetTransportStatus, false);


    // next set the value true
    iTransportTracker->updateState(Sync::CONNECTIVITY_USB, true);
    iTransportTracker->updateState(Sync::CONNECTIVITY_BT, true);
    iTransportTracker->updateState(Sync::CONNECTIVITY_INTERNET, true);

    usbTransportStatus = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_USB);
    QCOMPARE(usbTransportStatus, true);
    btTransportStatus = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_BT);
    QCOMPARE(btTransportStatus, true);
    internetTransportStatus = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_INTERNET);
    QCOMPARE(internetTransportStatus, true);

}

void TransportTrackerTest :: testStateChanged()
{
    // change USB state and verify

    iTransportTracker->onUsbStateChanged(false);
    QCOMPARE(iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_USB), false);


    // change BT state and verify
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    bool setBTState = false;
    QDBusVariant v(QVariant(true));
    iTransportTracker->onBtStateChanged("Powered", v);
#else
    bool setBTState = iTransportTracker->iDeviceInfo.currentBluetoothPowerState();
    iTransportTracker->onBtStateChanged(setBTState);
#endif
    QCOMPARE(iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_BT), setBTState);


    // change internet state and verify

    /* Memory allocation for iInternet is commented in TransportTracker.cpp, because of which
     * the below code core dumps. Once the memory allocation is done in the source file, the following
     * code needs to be uncommented
     */
/*
    bool setInternetState = iTransportTracker->iInternet->value().toBool();
    QVERIFY(iTransportTracker->iInternet);
    iTransportTracker->onInternetStateChanged();
    QCOMPARE(iTransportTracker->isConnectivityAvailable((Sync::CONNECTIVITY_INTERNET), setInternetState);
*/

    // Check signal emissions
    // register metatypes not known to QSignalSpy
     qRegisterMetaType<Sync::ConnectivityType>("Sync::ConnectivityType");

    QSignalSpy sampleSpy(iTransportTracker, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)));

    bool usbCurrentState = iTransportTracker->iTransportStates[Sync::CONNECTIVITY_USB];
    iTransportTracker->updateState((Sync::CONNECTIVITY_USB), !usbCurrentState);
    // check for the signal emission
    QCOMPARE(sampleSpy.count(), 1);

}



TESTLOADER_ADD_TEST(TransportTrackerTest);
