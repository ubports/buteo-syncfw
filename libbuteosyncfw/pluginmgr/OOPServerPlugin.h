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
#ifndef OOPSERVERPLUGIN_H
#define OOPSERVERPLUGIN_H

#include <ServerPlugin.h>
#include <QProcess>

namespace Buteo {
class OOPServerPlugin : public ServerPlugin
{
    Q_OBJECT

public:
    OOPServerPlugin( const QString& aPluginName,
                     const Profile& aProfile,
                     PluginCbInterface* aCbInterface,
                     QProcess& process );

    virtual ~OOPServerPlugin();

    virtual bool init();

    virtual bool uninit();

    virtual bool startListen();

    virtual void stopListen();

    virtual void suspend();

    virtual void resume();

    virtual bool cleanUp();

public slots:

    virtual void connectivityStateChanged( Sync::ConnectivityType aType,
                                           bool aState );
    void onProcessError(QProcess::ProcessError error);

    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onError(QString aProfileName, QString aMessage, int aErrorCode);

    void onSuccess(QString aProfileName, QString aMessage);

private:
    bool iDone;
};

}
#endif // OOPSERVERPLUGIN_H
