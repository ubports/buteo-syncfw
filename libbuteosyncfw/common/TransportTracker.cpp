/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *               2019 Updated to use bluez5 by deloptes@gmail.com
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

#include "TransportTracker.h"
#if __USBMODED__
#include "USBModedProxy.h"
#endif
#include "NetworkManager.h"
#include "LogMacros.h"
#include <QMutexLocker>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusArgument>

using namespace Buteo;

TransportTracker::TransportTracker(QObject *aParent) :
    QObject(aParent),
    iUSBProxy(0),
    iInternet(0),
    iSystemBus(QDBusConnection::systemBus())
{
    FUNCTION_CALL_TRACE;

    iTransportStates[Sync::CONNECTIVITY_USB] = false;
    iTransportStates[Sync::CONNECTIVITY_BT] = false;
    iTransportStates[Sync::CONNECTIVITY_INTERNET] = false;

#if __USBMODED__
    // USB
    iUSBProxy = new USBModedProxy(this);
    if (!iUSBProxy->isValid()) {
        LOG_CRITICAL("Failed to connect to USB moded D-Bus interface");
        delete iUSBProxy;
        iUSBProxy = nullptr;
    } else {
        QObject::connect(iUSBProxy, SIGNAL(usbConnection(bool)), this,
                         SLOT(onUsbStateChanged(bool)));
        iTransportStates[Sync::CONNECTIVITY_USB] =
            iUSBProxy->isUSBConnected();
    }
#endif

#ifdef HAVE_BLUEZ_5
    // BT
    qDBusRegisterMetaType <InterfacesMap> ();
    qDBusRegisterMetaType <ObjectsMap> ();

    // listen for added interfaces
    if (!iSystemBus.connect(BT::BLUEZ_DEST,
                     QString("/"),
                     BT::BLUEZ_MANAGER_INTERFACE,
                     BT::INTERFACESADDED,
                     this,
                     SLOT(onBtInterfacesAdded(QDBusObjectPath, InterfacesMap)))) {
        LOG_WARNING("Failed to connect InterfacesAdded signal");
    }

    if (!iSystemBus.connect(BT::BLUEZ_DEST,
                     QString("/"),
                     BT::BLUEZ_MANAGER_INTERFACE,
                     BT::INTERFACESREMOVED,
                     this,
                     SLOT(onBtInterfacesRemoved(QDBusObjectPath, QStringList)))) {
        LOG_WARNING("Failed to connect InterfacesRemoved signal");
    }

    // get the initial state
    if (btConnectivityStatus()) {
        if (!iSystemBus.connect(BT::BLUEZ_DEST,
                iDefaultBtAdapter,
                BT::BLUEZ_PROPERTIES_INTERFACE,
                BT::PROPERTIESCHANGED,
                this,
                SLOT(onBtStateChanged(QString, QVariantMap, QStringList)))) {
            LOG_WARNING("Failed to connect PropertiesChanged signal");
        }
        // Set the bluetooth state to on
        iTransportStates[Sync::CONNECTIVITY_BT] = true;
    } else {
        LOG_WARNING("The BT adapter is powered off or missing");
    }
#endif

    // Internet
    // @todo: enable when internet state is reported correctly.
    iInternet = new NetworkManager(this);
    iTransportStates[Sync::CONNECTIVITY_INTERNET] =
            iInternet->isOnline();
    connect(iInternet,
            SIGNAL(statusChanged(bool, Sync::InternetConnectionType)),
            SLOT(onInternetStateChanged(bool, Sync::InternetConnectionType)) /*, Qt::QueuedConnection*/);
}

TransportTracker::~TransportTracker()
{
    FUNCTION_CALL_TRACE;
}

bool TransportTracker::isConnectivityAvailable(Sync::ConnectivityType aType) const
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    return iTransportStates[aType];
}

void TransportTracker::onUsbStateChanged(bool aConnected)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("USB state changed:" << aConnected);
    updateState(Sync::CONNECTIVITY_USB, aConnected);
}

#ifdef HAVE_BLUEZ_5
void TransportTracker::onBtStateChanged(QString interface, QVariantMap changed, QStringList invalidated)
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(invalidated);

    if (interface == BT::BLUEZ_ADAPTER_INTERFACE) {
        for (QVariantMap::iterator i = changed.begin(); i != changed.end(); ++i) {
            if (i.key() == "Powered") {
                bool btOn = i.value().toBool();
                LOG_INFO("BT power state " << btOn);
                updateState(Sync::CONNECTIVITY_BT, btOn);
            }
        }
    }
}

void TransportTracker::onBtInterfacesAdded(const QDBusObjectPath &path, const InterfacesMap interfaces)
{
    FUNCTION_CALL_TRACE;

    for (InterfacesMap::const_iterator i = interfaces.cbegin(); i != interfaces.cend(); ++i) {
        if (i.key() == BT::BLUEZ_ADAPTER_INTERFACE) {

            // do not process other interfaces after default one was selected
            if (!iDefaultBtAdapter.isEmpty())
                break;

            iDefaultBtAdapter = path.path();
            LOG_DEBUG(BT::BLUEZ_ADAPTER_INTERFACE << "interface" << iDefaultBtAdapter);

            QDBusInterface adapter(BT::BLUEZ_DEST,
                    iDefaultBtAdapter,
                    BT::BLUEZ_ADAPTER_INTERFACE,
                    iSystemBus);

            if (!iSystemBus.connect(BT::BLUEZ_DEST,
                    iDefaultBtAdapter,
                    BT::BLUEZ_PROPERTIES_INTERFACE,
                    BT::PROPERTIESCHANGED,
                    this,
                    SLOT(onBtStateChanged(QString, QVariantMap, QStringList)))) {
                LOG_WARNING("Failed to connect PropertiesChanged signal");

            }

            if (adapter.isValid()) {
                updateState(Sync::CONNECTIVITY_BT, adapter.property("Powered").toBool());
                LOG_INFO("BT state changed" << adapter.property("Powered").toBool());
            }
        }
    }
}

void TransportTracker::onBtInterfacesRemoved(const QDBusObjectPath &path, const QStringList interfaces)
{
    FUNCTION_CALL_TRACE;

    for (QStringList::const_iterator i = interfaces.cbegin(); i != interfaces.cend(); ++i) {
        if (*i == BT::BLUEZ_ADAPTER_INTERFACE) {

            if (path.path() != iDefaultBtAdapter)
                continue;

            LOG_DEBUG("DBus adapter path: " << iDefaultBtAdapter );

            if (!iSystemBus.disconnect(BT::BLUEZ_DEST,
                    iDefaultBtAdapter,
                    BT::BLUEZ_PROPERTIES_INTERFACE,
                    BT::PROPERTIESCHANGED,
                    this,
                    SLOT(onBtStateChanged(QString,QVariantMap,QStringList)))) {
                LOG_WARNING("Failed to disconnect PropertiesChanged signal");
            } else {
                LOG_DEBUG("'org.bluez.Adapter1' interface removed from " << path.path());
            }

            iDefaultBtAdapter = QString();

            break;
        }
    }
}
#endif

void TransportTracker::onInternetStateChanged(bool aConnected, Sync::InternetConnectionType aType)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Internet state changed:" << aConnected);
    updateState(Sync::CONNECTIVITY_INTERNET, aConnected);
    emit networkStateChanged(aConnected, aType);
}

void TransportTracker::updateState(Sync::ConnectivityType aType, bool aState)
{
    FUNCTION_CALL_TRACE;


    bool oldState = false;
    {
        QMutexLocker locker(&iMutex);
        oldState = iTransportStates[aType];
        iTransportStates[aType] = aState;
    }
    if (oldState != aState) {
        if (aType != Sync::CONNECTIVITY_INTERNET) {
            emit connectivityStateChanged(aType, aState);
        }
    }
}

#ifdef HAVE_BLUEZ_5
bool TransportTracker::btConnectivityStatus()
{
    FUNCTION_CALL_TRACE;

    QDBusInterface  manager(BT::BLUEZ_DEST,
            QString("/"),
            BT::BLUEZ_MANAGER_INTERFACE,
            iSystemBus);

    QDBusReply<ObjectsMap> reply = manager.call(BT::GETMANAGEDOBJECTS);
    if (!reply.isValid()) {
        LOG_WARNING( "Failed to connect BT ObjectManager: " << reply.error().message() );
        return false;
    }

    ObjectsMap objects = reply.value();
    for (ObjectsMap::iterator i = objects.begin(); i != objects.end(); ++i) {

        InterfacesMap ifaces = i.value();
        for (InterfacesMap::const_iterator j = ifaces.cbegin(); j != ifaces.cend(); ++j) {

            if (j.key() == BT::BLUEZ_ADAPTER_INTERFACE) {
                if (iDefaultBtAdapter.isEmpty() || iDefaultBtAdapter != i.key().path()) {
                    iDefaultBtAdapter = i.key().path();
                    LOG_DEBUG ("Using adapter path: " << iDefaultBtAdapter);
                }
                QDBusInterface adapter(BT::BLUEZ_DEST,
                        iDefaultBtAdapter,
                        BT::BLUEZ_ADAPTER_INTERFACE,
                        iSystemBus);

                return adapter.property("Powered").toBool(); // use first adapter
            }
        }
    }

    return false;
}
#endif
