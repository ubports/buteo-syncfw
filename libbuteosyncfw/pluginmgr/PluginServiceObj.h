/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
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

#include <QObject>
#include <QString>
#include <Profile.h>
#include <SyncProfile.h>
#include <PluginCbImpl.h>

#include CLASSNAME_H

using namespace Buteo;

class PluginServiceObj : public QObject
{
    Q_OBJECT
public:
    PluginServiceObj( QString aProfile, QString aPluginName, QObject *parent = 0 );
    virtual ~PluginServiceObj();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void abortSync(uchar aStatus);
    bool cleanUp();
    void connectivityStateChanged(int aType, bool aState);
    QString getSyncResults();
    bool init();
    bool uninit();
#ifdef CLIENT_PLUGIN
    bool startSync();
#else
    void resume();
    bool startListen();
    void stopListen();
    void suspend();
#endif
Q_SIGNALS: // SIGNALS
    void accquiredStorage(const QString &aMimeType);
    void error(const QString &aProfileName, const QString &aMessage, int aErrorCode);
    void newSession(const QString &aDestination);
    void success(const QString &aProfileName, const QString &aMessage);
    void syncProgressDetail(const QString &aProfileName, int aProgressDetail);
    void transferProgress(const QString &aProfileName, int aTransferDatabase, int aTransferType, const QString &aMimeType, int aCommittedItems);

private:
    CLASSNAME      *iPlugin;
    QString        iProfileName;
    QString        iPluginName;
    PluginCbImpl   iPluginCb;
};

#endif // PLUGINSERVICEOBJ_H
