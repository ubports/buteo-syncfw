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

#include "SyncDBusConnection.h"
#include  "LogMacros.h"
#include <QThreadStorage>

namespace Buteo {

    //! Private DBusConnection object that Auto Disconnects when the storage
    // for this object is being released.
    class AutoDBusConnection : public QDBusConnection
    {
    public:
        AutoDBusConnection(const QDBusConnection& c)
        : QDBusConnection(c)
        {
            FUNCTION_CALL_TRACE;
        }
        ~AutoDBusConnection()
        {
            disconnectFromBus(name());
        }
    };

    // Thread Specific Storage to store the dbus connection to session bus for this thread
    QThreadStorage<AutoDBusConnection*> sessionBusConnection;

    // Thread Specific Storage to store the dbus connection to system bus for this thread
    QThreadStorage<AutoDBusConnection*> systemBusConnection;

    // Global Variables to give different connection names in different threads.
    QAtomicInt sessionBusCounter = 0;
    QAtomicInt systemBusCounter = 0;


    QDBusConnection SyncDBusConnection::sessionBus()
    {
        FUNCTION_CALL_TRACE;
        // Create a separate D-Bus connection to Session Bus for each thread.
        // Use AutoDBusConnection so that the bus gets disconnected when the
        // thread storage is deleted.
        if (!sessionBusConnection.hasLocalData()) {
            QString connectionName = QString::number(sessionBusCounter.fetchAndAddRelaxed(1))
                    .prepend("sync-session-bus-");
            sessionBusConnection.setLocalData(new AutoDBusConnection
                    (QDBusConnection::connectToBus(QDBusConnection::SessionBus,connectionName)));
        }

#ifndef QT_NO_DEBUG
        // creating a local variable only for debugging problems with strange backtraces.
        QDBusConnection dbus = *sessionBusConnection.localData();
        //warning level only to be sure this is printed always
        LOG_DEBUG("DBus Connection Address:" << &dbus );
        LOG_DEBUG("DBus Connection Name:" << dbus.name() );
        LOG_DEBUG("DBus is Connected:" << dbus.isConnected() );
        LOG_DEBUG("DBus Connection ID:" << dbus.baseService() );
#endif

        return *sessionBusConnection.localData();
    }

    QDBusConnection SyncDBusConnection::systemBus()
    {
        FUNCTION_CALL_TRACE;
        // Create a separate D-Bus connection to System Bus for each thread.
        // Use SyncDBusConnection so that the bus gets disconnected when the
        // thread storage is deleted.
        if (!systemBusConnection.hasLocalData()) {
            QString connectionName = QString::number(systemBusCounter.fetchAndAddRelaxed(1))
                    .prepend("sync-system-bus-");
            systemBusConnection.setLocalData(new AutoDBusConnection
                    (QDBusConnection::connectToBus(QDBusConnection::SystemBus,connectionName)));
        }

#ifndef QT_NO_DEBUG
        // creating a local variable only for debugging problems with strange backtraces.
        QDBusConnection dbus = *systemBusConnection.localData();
        //warning level only to be sure this is printed always
        LOG_DEBUG("DBus Connection Address:" << &dbus );
        LOG_DEBUG("DBus Connection Name:" << dbus.name() );
        LOG_DEBUG("DBus is Connected:" << dbus.isConnected() );
        LOG_DEBUG("DBus Connection ID:" << dbus.baseService() );
#endif

        return *systemBusConnection.localData();
    }


};


