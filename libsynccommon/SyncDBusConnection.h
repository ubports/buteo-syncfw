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
