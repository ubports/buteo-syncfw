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

#ifndef OOPPLUGINMANAGER_H
#define OOPPLUGINMANAGER_H

#include <QMap>
#include <QReadWriteLock>
#include <QString>

#include "PluginManager.h"

namespace Buteo {

class SyncProfile;
class Profile;
class OOPClientPlugin;
class OOPServerPlugin;

// Binary names suffix
const QString OOP_CLIENT_SUFFIX = "-client";
const QString OOP_SERVER_SUFFIX = "-server";
const QString DEFAULT_OOP_PATH = "/usr/lib/buteo-plugins-qt5/oopp";

class OOPPluginManager : public PluginManager
{
public:
    OOPPluginManager();

    ~OOPPluginManager();

    OOPClientPlugin* createClient( const QString& aPluginName,
                                   const SyncProfile& aProfile );

    void destroyClient( OOPClientPlugin* aPlugin );

    OOPServerPlugin* createServer( const QString& aPluginName,
                                   const Profile& aProfile );

    void destroyServer( OOPServerPlugin* aPlugin );

private:
 
    struct ProcInfo
    {
        QString   iPath;
        void*     iHandle;
        int       iRefCount;

        ProcInfo() : iHandle( NULL ), iRefCount( 0 ) { }
    };

    void* startOOPPlugin( const QString aFilter, QMap<QString, QString> &aTargetMap );

    void* getProcHandle( const QString& aPath );

    void stopOOPPlugin( const QString& aPath );

    QString                  iPluginPath;

    QMap<QString, QString>   iClientMaps;

    QMap<QString, QString>   iServerMaps;

    QList<ProcInfo>          iStartedProcs;

    QReadWriteLock           iProcLock;
};

}

#endif // OOPPLUGINMANAGER_H
