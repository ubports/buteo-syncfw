/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 - 2021 Jolla Ltd.
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
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
*/
#ifndef PLUGINSERVICEOBJ_H
#define PLUGINSERVICEOBJ_H

#include "PluginCbImpl.h"

#include <Profile.h>
#include <SyncProfile.h>
#include <SyncCommonDefs.h>
#include <SyncPluginLoader.h>

class QPluginLoader;

using namespace Buteo;

class PluginServiceObj : public QObject
{
    Q_OBJECT
public:
    PluginServiceObj(const QString &aPluginName,
                     const QString &aProfileName,
                     const QString &aPluginFilePath,
                     QObject *parent = nullptr);
    virtual ~PluginServiceObj();

public Q_SLOTS:
    void abortSync(uchar aStatus);
    bool cleanUp();
    void connectivityStateChanged(int aType, bool aState);
    QString getSyncResults();
    bool init();
    bool uninit();

    // client functions
    bool startSync();

    // server functions
    void resume();
    bool startListen();
    void stopListen();
    void suspend();

Q_SIGNALS:
    void accquiredStorage(const QString &aMimeType);
    void error(const QString &aProfileName, const QString &aMessage, int aErrorCode);
    void newSession(const QString &aDestination);
    void success(const QString &aProfileName, const QString &aMessage);
    void syncProgressDetail(const QString &aProfileName, int aProgressDetail);
    void transferProgress(const QString &aProfileName, Sync::TransferDatabase aDatabase, Sync::TransferType aType, const QString &aMimeType, int aCommittedItems);

private:
    SyncPluginBase *initializePlugin();

    QPluginLoader *iPluginLoader = nullptr;
    SyncPluginLoader *iSyncPluginLoader = nullptr;
    QPointer<SyncPluginBase> iPlugin = nullptr;
    Buteo::PluginCbImpl *iPluginCb = nullptr;
    QString iPluginName;
    QString iProfileName;
    QString iPluginFilePath;
};

#endif // PLUGINSERVICEOBJ_H
