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
#ifndef OOPCLIENTPLUGIN_H
#define OOPCLIENTPLUGIN_H

#include <ClientPlugin.h>
#include <QProcess>

namespace Buteo {

class OOPClientPlugin : public ClientPlugin
{
public:
    OOPClientPlugin( const QString& aPluginName,
                     const Buteo::SyncProfile& aProfile,
                     Buteo::PluginCbInterface* aCbInterface,
                     QProcess& aProcess);

    virtual ~OOPClientPlugin();

    virtual bool init();

    virtual bool uninit();

    virtual bool startSync();

    virtual void abortSync(Sync::SyncStatus aStatus = Sync::SYNC_ABORTED);

    virtual Buteo::SyncResults getSyncResults() const;

    virtual bool cleanUp();

public slots:

    virtual void connectivityStateChanged(Sync::ConnectivityType aType,
                                          bool aState);
};

}

#endif // OOPCLIENTPLUGIN_H
