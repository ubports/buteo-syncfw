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
#ifndef SYNCSCHEDULERTEST_H_
#define SYNCSCHEDULERTEST_H_

#include <QtTest/QtTest>
#include <QDateTime>
#include "SyncProfile.h"

namespace Buteo {

class SyncScheduler;

class SyncSchedulerTest: public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();
    void syncTriggered(QString aProfileName);

    void testAddRemoveProfile();
    void testSetNextAlarm();

private:

    SyncScheduler *iSyncScheduler;
    QString        iSyncProfileName;
};

class SyncProfileStub : public SyncProfile
{

public:
    SyncProfileStub(const QString &aName) : SyncProfile(aName) {}
    ~SyncProfileStub() {}

    QDateTime nextSyncTime() const
    {
        const int HOUR = 3600;
        int syncTime_t = QDateTime::currentDateTime().toTime_t() + HOUR;
        return QDateTime::fromTime_t(syncTime_t);
    }

};

}
#endif // SYNCSCHEDULERTEST_H_

