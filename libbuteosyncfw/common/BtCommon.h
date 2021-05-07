/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2020 as part of an update to use bluez5 by deloptes@gmail.com
 *
 */

#ifndef BTCOMMON_H_
#define BTCOMMON_H_

#include <QtDBus>
#include <QMap>

#if HAVE_BLUEZ_5
namespace Buteo {

namespace BT {
    static const QString BLUEZ_DEST = "org.bluez";
    static const QString BLUEZ_MANAGER_INTERFACE = "org.freedesktop.DBus.ObjectManager";
    static const QString BLUEZ_ADAPTER_INTERFACE = "org.bluez.Adapter1";
    static const QString BLUEZ_DEVICE_INTERFACE = "org.bluez.Device1";
    static const QString BLUEZ_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
    static const QString GETMANAGEDOBJECTS = "GetManagedObjects";
    static const QString GETPROPERTIES = "GetAll";
    static const QString PROPERTIESCHANGED = "PropertiesChanged";
    static const QString INTERFACESADDED = "InterfacesAdded";
    static const QString INTERFACESREMOVED = "InterfacesRemoved";

} // namespace BT

typedef QMap<QString,QVariantMap> InterfacesMap;
typedef QMap<QDBusObjectPath,InterfacesMap> ObjectsMap;

} // namespace Buteo

Q_DECLARE_METATYPE(Buteo::InterfacesMap)
Q_DECLARE_METATYPE(Buteo::ObjectsMap)

#endif /* HAVE_BLUEZ_5 */

#endif /* BTCOMMON_H_ */
