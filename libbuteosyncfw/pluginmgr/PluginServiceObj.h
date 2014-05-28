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

#include CLASSNAME_H

using namespace Buteo;

class PluginServiceObj : public QObject
{
    Q_OBJECT
public:
    explicit PluginServiceObj(QObject *parent = 0);
    PluginServiceObj( QString aProfile, QString aPluginName, QObject *parent = 0 );
    virtual ~PluginServiceObj();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void abortSync(uchar aStatus);
    bool cleanUp();
    void connectivityStateChanged(int aType, int aState);
    QString getPluginName();
    QString getProfileName();
    QString getSyncResults();
    bool init();
    QString profile();
    void resume();
    void setPluginParams(const QString &aPluginName, const QString &aProfileName);
    bool startListen();
    void startSync();
    void stopListen();
    void suspend();
    bool uninit();
    void exitWithSyncSuccess(QString aProfileName, QString aState);
    void exitWithSyncFailed(QString aProfileName, QString aMessage, int aErrorCode);
Q_SIGNALS: // SIGNALS
    void accquiredStorage(const QString &aMimeType);
    void error(const QString &aProfileName, const QString &aMessage, int aErrorCode);
    void newSession(const QString &aDestination);
    void success(const QString &aProfileName, const QString &aMessage);
    void syncProgressDetail(const QString &aProfileName, int aProgressDetail);
    void transferProgress(const QString &aProfileName, int aTransferDatabase, int aTransferType, const QString &aMimeType, int aCommittedItems);

    // Signal to indicate to the main() function of sync done 
    void syncDone(const Buteo::SyncProfile::CurrentSyncStatus status);
    
private:
    SyncProfile    *iProfile;
    QString         iPluginName;
    CLASSNAME      *iPlugin;
};

#endif // PLUGINSERVICEOBJ_H
