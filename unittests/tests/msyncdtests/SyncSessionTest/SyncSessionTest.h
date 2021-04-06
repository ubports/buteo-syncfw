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


#ifndef SYNCSESSIONTEST_H
#define SYNCSESSIONTEST_H

#include <QPointer>

#include "SyncResults.h"
#include "StorageBooker.h"
#include "SyncProfile.h"
#include "PluginRunner.h"

namespace Buteo {

class SyncSession;
class SyncSessionPluginRunnerTest;

class SyncSessionTest : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void testPluginRunner();
    void testStartAbortStop();
    void testProfile();
    void testScheduled();
    void testResults();
    void testStorages();
    void testOnSuccess();
    void testOnError();
    void testOnTransferProgress();
    void testOnDone();

private:

    SyncSession *iSyncSession;
    SyncProfile *iSyncProfile;
    PluginRunner *iNullPluginRunner;
    QPointer<SyncSessionPluginRunnerTest> iSyncSessionPluginRunnerTest;

public:
    static bool isValuePassedTrue;

};


class SyncSessionPluginRunnerTest : public PluginRunner
{
    Q_OBJECT
public slots:
    bool init();
    bool start();
    void stop();
    void abort(Sync::SyncStatus aStatus = Sync::SYNC_ABORTED);
    bool cleanUp();
    SyncResults syncResults();
    SyncPluginBase *plugin();

public:
    SyncSessionPluginRunnerTest(const QString &aPluginName,
                                PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf);

private:
    PluginRunner *iPluginRunner;

public:
    static int testValue; // to cross-check the value while calling stop() / abort()
};

}

#endif // SYNCSESSIONTEST_H
