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
#include "IPHeartBeatTest.h"
#include "IPHeartBeat.h"

using namespace Buteo;

void IPHeartBeatTest::initTestCase()
{
    iHbeat = new IPHeartBeat(this);
    connect(iHbeat,SIGNAL(onHeartBeat(QString)),this,SLOT(onBeatTriggered(QString)));
}

void IPHeartBeatTest::cleanupTestCase()
{
    delete iHbeat;
}

void IPHeartBeatTest::testSetHeartBeat()
{
    iBeatReceived = false;
    if(iHbeat->setHeartBeat("someprofile",0,2) == true) {
        QTest::qWait(1000*3);
        QVERIFY(iBeatReceived == true);
    } else {
        // If iphbd service is not running setHeartBeat() will fail
        // Not letting that to make the test to fail
        qDebug() << "******* Start the iphbd service and run this test *******";
    }
}

void IPHeartBeatTest::testRemoveHeartBeat()
{
    iHbeat->setHeartBeat("someprofile",0,20);

    iHbeat->removeWait("someprofile");

    QVERIFY(iHbeat->iBeatsWaiting.contains("someprofile") == false);
}

void IPHeartBeatTest::testRemoveAllHeartBeats()
{
    iHbeat->setHeartBeat("someprofile",0,20);
    iHbeat->setHeartBeat("anotherprofile",50,100);

    iHbeat->removeAllWaits();

    QVERIFY(iHbeat->iBeatsWaiting.size() == 0);
}

void IPHeartBeatTest::testInternalBeat()
{
    iHbeat->internalBeatTriggered(-1);
}

void IPHeartBeatTest::onBeatTriggered(QString /*aProfName*/)
{
    iBeatReceived = true;
}

QTEST_MAIN(Buteo::IPHeartBeatTest)
