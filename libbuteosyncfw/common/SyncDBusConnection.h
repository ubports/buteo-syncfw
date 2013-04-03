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

#ifndef SYNCDBUSCONNECTION_H
#define SYNCDBUSCONNECTION_H

#include <QDBusConnection>

namespace Buteo {

// this is a workaround for QDBusConnection::SessionBus as it is not thread safe.
// Use this class as an alternative to QDBusConnection to get a thread private
// dbus connection
class SyncDBusConnection {

public:

    //! static function to retrieve a dbus connection to sessionBus
    static QDBusConnection sessionBus();

    //! static function to retrieve a dbus connection to systemBus
    static QDBusConnection systemBus();

private:
    //! private constructor.
    // Cannot Create Objects of this type
    SyncDBusConnection() { }

    //! destructor
    ~SyncDBusConnection() { }
};

}

#endif
