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
    qRegisterMetaType<Sync::ConnectivityType>("Sync::ConnectivityType");
    QSignalSpy connectivityStateSpy(iTransportTracker, SIGNAL(connectivityStateChanged(Sync::ConnectivityType, bool)));
    QSignalSpy networkStateSpy(iTransportTracker, SIGNAL(networkStateChanged(bool,Sync::InternetConnectionType)));

    // change USB state and verify
    bool usbCurrentState = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_USB);
    iTransportTracker->onUsbStateChanged(!usbCurrentState);
    QCOMPARE(iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_USB), !usbCurrentState);
    QCOMPARE(connectivityStateSpy.count(), 1);
    QCOMPARE(connectivityStateSpy.first().at(0).value<Sync::ConnectivityType>(), Sync::CONNECTIVITY_USB);
    QCOMPARE(connectivityStateSpy.first().at(1).value<bool>(), !usbCurrentState);
    connectivityStateSpy.clear();

    // change BT state and verify
    bool btCurrentState = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_BT);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    iTransportTracker->onBtStateChanged("Powered", QDBusVariant(QVariant(!btCurrentState)));
#else
    iTransportTracker->onBtStateChanged(!btCurrentState);
#endif
    QCOMPARE(iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_BT), !btCurrentState);
    QCOMPARE(connectivityStateSpy.count(), 1);
    QCOMPARE(connectivityStateSpy.first().at(0).value<Sync::ConnectivityType>(), Sync::CONNECTIVITY_BT);
    QCOMPARE(connectivityStateSpy.first().at(1).value<bool>(), !btCurrentState);
    connectivityStateSpy.clear();

    // change internet state and verify
    bool internetCurrentState = iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_INTERNET);
    iTransportTracker->onInternetStateChanged(!internetCurrentState, Sync::INTERNET_CONNECTION_UNKNOWN);
    QCOMPARE(iTransportTracker->isConnectivityAvailable(Sync::CONNECTIVITY_INTERNET), !internetCurrentState);
    QEXPECT_FAIL("", "IMO connectivityStateChanged() should be emitted also for CONNECTIVITY_INTERNET", Continue);
    QCOMPARE(connectivityStateSpy.count(), 1);
    //QCOMPARE(connectivityStateSpy.first().at(0).value<Sync::ConnectivityType>(), Sync::CONNECTIVITY_INTERNET);
    //QCOMPARE(connectivityStateSpy.first().at(1).value<bool>(), !internetCurrentState);
    //connectivityStateSpy.clear();
    QCOMPARE(networkStateSpy.count(), 1);
    QCOMPARE(networkStateSpy.first().at(0).value<bool>(), !internetCurrentState);
    networkStateSpy.clear();
}



QTEST_MAIN(Buteo::TransportTrackerTest)
