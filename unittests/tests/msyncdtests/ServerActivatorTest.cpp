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



#include "ServerActivatorTest.h"
#include <SyncCommonDefs.h>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include "Profile.h"
#include "SyncFwTestLoader.h"

#include <QStringList>
#include <QDomDocument>

using namespace Buteo;

void ServerActivatorTest::initTestCase()
{
    iTransportTracker = new TransportTracker(this);
    iProfileManager = new ProfileManager("profile1", "profile2");

    // add server profiles through Profile class
    Profile myProfile("sampleServerProfile", Profile::TYPE_SERVER);
    QVERIFY(iProfileManager->save(myProfile));

    Profile myProfile2("sampleServerProfile2", Profile::TYPE_SERVER);
    iProfileManager->save(myProfile2);

    iServerActivator = new ServerActivator(*iProfileManager, *iTransportTracker, this);

    // iServers.keys() does the same
    QVERIFY(iServerActivator->iServers.count());

}


void ServerActivatorTest::cleanupTestCase()
{
    //  deallocate the memory
    delete iServerActivator;

    delete iProfileManager;
    delete iTransportTracker;
}

void ServerActivatorTest :: testRef()
{
    // dummy server name which fails
    const QString SERVERNAME = "dummy";
    // saved server profile name
    const QString ANOTHERSERVER = "sampleServerProfile";

    QSignalSpy enabledSpy(iServerActivator, SIGNAL(serverEnabled(QString)));
    QSignalSpy disabledSpy(iServerActivator, SIGNAL(serverDisabled(QString)));

    int returnedVal = iServerActivator->addRef(SERVERNAME);
    // this call should fail as there is no "dummy" profile saved. so compare with expected value '0'
    QCOMPARE(returnedVal, 0);
    QCOMPARE(enabledSpy.count(), 0);

    returnedVal = iServerActivator->addRef(ANOTHERSERVER);
    // when the server name is found, ref count will be incremented and returned to the calling function
    QCOMPARE(returnedVal, 1);
    QCOMPARE(enabledSpy.count(), 1);

    returnedVal = iServerActivator->removeRef(SERVERNAME);
    QCOMPARE(returnedVal, 0);

    returnedVal = iServerActivator->removeRef(ANOTHERSERVER);
    QCOMPARE(returnedVal, 0);
    QCOMPARE(disabledSpy.count(), 1);
    // signals will be emitted only if reference count is 1 for addRef and 0 for removeRef

}


void ServerActivatorTest :: testEnabledServers()
{
    // test enableServers() without adding server name. expected result is empty stringlist
    QStringList serverData = iServerActivator->enabledServers();
    QVERIFY(serverData.empty() != 0);

    // add a servername and test the function again(). expected result is added
    // server name as stringlist returned
    iServerActivator->addRef("sampleServerProfile2");
    serverData = iServerActivator->enabledServers();
    QVERIFY(serverData.count() && serverData.contains("sampleServerProfile2"));
}

void ServerActivatorTest :: testConnectivityStateChanged()
{
    // adding a server profile and use USB transport medium

    const QString SERVER_XML=
            " <profile name=\"usbserver\" type=\"server\" > "
                " <key name=\"usb_transport\" value=\"true\"/> "
            " </profile> ";
    QDomDocument doc;

    QVERIFY(doc.setContent(SERVER_XML, false));
    Profile sampleServerProfile(doc.documentElement());
    sampleServerProfile.setName("sampleProfile");
    const QString PROFILE_PATH("syncprofiletests/testprofiles/user");
    ProfileManager myProfileManager(PROFILE_PATH, PROFILE_PATH);
    myProfileManager.save(sampleServerProfile);
    TransportTracker myTrasportTracker(this);

    ServerActivator sampleServerActivator(myProfileManager, myTrasportTracker, this);

    QSignalSpy enabledSpy(&sampleServerActivator, SIGNAL(serverEnabled(QString)));
    QSignalSpy disabledSpy(&sampleServerActivator, SIGNAL(serverDisabled(QString)));

    // make sure that the present count is '0'
    QCOMPARE(enabledSpy.count(), 0);
    sampleServerActivator.onConnectivityStateChanged(Sync::CONNECTIVITY_USB, true);
    // after calling onConnectivityStateChanged(), total signal emissions will be total number
    // of server profiles available.
    QCOMPARE(enabledSpy.count(), 1);

    // make sure that the present count is '0'
    QCOMPARE(disabledSpy.count(), 0);
    sampleServerActivator.onConnectivityStateChanged(Sync::CONNECTIVITY_USB, false);
    QCOMPARE(disabledSpy.count(), 1);

    QVERIFY(myProfileManager.remove("sampleProfile", Profile::TYPE_SERVER));
}

void ServerActivatorTest :: testTransportsFromProfile()
{
    Profile profileToSetKey("profile", Profile::TYPE_SERVER);
    profileToSetKey.setKey("profilekey", "keyval");

    QList<Sync::ConnectivityType> typeReceived = iServerActivator->transportsFromProfile(&profileToSetKey);
    QVERIFY(typeReceived.empty() != 0);

    // XML profiles creation
    const QString USB_XML=
               " <profile name=\"usbserver\" type=\"server\" > "
                " <key name=\"usb_transport\" value=\"true\"/> "
               " </profile> ";
    const QString BT_XML=
              " <profile name=\"btserver\" type=\"server\" > "
                " <key name=\"bt_transport\" value=\"true\"/> "
              " </profile> ";
    const QString INTERNET_XML=
              " <profile name=\"internetserver\" type=\"server\" > "
                " <key name=\"internet_transport\" value=\"true\"/> "
              " </profile> ";


    // test for USB transport
    QDomDocument doc;
    QVERIFY(doc.setContent(USB_XML, false));
    Profile usbServerProfile(doc.documentElement());
    typeReceived = iServerActivator->transportsFromProfile(&usbServerProfile);
    QCOMPARE(typeReceived.count(), 1);
    QCOMPARE(typeReceived.takeFirst(), Sync::CONNECTIVITY_USB);

    // test for BT transport
    QVERIFY(doc.setContent(BT_XML, false));
    Profile btServerProfile(doc.documentElement());
    typeReceived = iServerActivator->transportsFromProfile(&btServerProfile);
    QCOMPARE(typeReceived.count(), 1);
    QCOMPARE(typeReceived.takeFirst(), Sync::CONNECTIVITY_BT);

    // test for internet transport
    QVERIFY(doc.setContent(INTERNET_XML, false));
    Profile internetServerProfile(doc.documentElement());
    typeReceived = iServerActivator->transportsFromProfile(&internetServerProfile);
    QCOMPARE(typeReceived.count(), 1);
    QCOMPARE(typeReceived.takeFirst(), Sync::CONNECTIVITY_INTERNET);
}


TESTLOADER_ADD_TEST(ServerActivatorTest);
