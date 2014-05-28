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
#include <QCoreApplication>
#include <QDBusConnection>
#include "PluginServiceObj.h"
#include "ButeoPluginIfaceAdaptor.h"
#include "LogMacros.h"

#define DBUS_SERVICE_NAME_PREFIX "com.buteo.msyncd.plugin."
#define DBUS_SERVICE_OBJ_PATH "/"

int main( int argc, char** argv )
{
    QCoreApplication app( argc, argv );

    // We obtain the plugin name and the profile name from cmdline
    // One way to pass the arguments is via cmdline, the other way is
    // to use the method setPluginParams() dbus method. But setting
    // cmdline arguments is probably cleaner
    if( (argc != 3) || (argv[1] == NULL) || (argv[2] == NULL) )
    {
        LOG_FATAL( "Plugin name and profile name are not obtained from cmdline" );
    }
    QString pluginName = QString( argv[1] );
    QString profileName = QString( argv[2] );

#ifndef CLASSNAME
    LOG_FATAL( "CLASSNAME value not defined in project file" );
#endif

    PluginServiceObj *serviceObj = new PluginServiceObj( profileName, pluginName );
    if( !serviceObj ) {
        LOG_FATAL( "Unable to create the service adaptor object" );
    }

    new ButeoPluginIfaceAdaptor( serviceObj );

    // Create a session bus
    QDBusConnection connection = QDBusConnection::sessionBus();
    if( connection.registerService( DBUS_SERVICE_NAME_PREFIX + profileName ) == true ) {
        if( connection.registerObject(DBUS_SERVICE_OBJ_PATH, serviceObj) == true ) {
            LOG_DEBUG( "Plugin " << pluginName << " with profile "
                       << profileName << " registered at dbus "
                       << DBUS_SERVICE_NAME_PREFIX + profileName
                       << " and path " << DBUS_SERVICE_OBJ_PATH );
        } else {
            LOG_FATAL( "Unable to register dbus service" );
        }
    } else {
        LOG_FATAL( "Unable to register service" );
    }

    // TODO: Should any unix signals be handled?
    return app.exec();
}
