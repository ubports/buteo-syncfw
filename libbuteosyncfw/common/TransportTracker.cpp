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
    iSystemBus(0)
{
    FUNCTION_CALL_TRACE;

    iTransportStates[Sync::CONNECTIVITY_USB] = false;
    iTransportStates[Sync::CONNECTIVITY_BT] = false;
    iTransportStates[Sync::CONNECTIVITY_INTERNET] = false;

#if __USBMODED__
    // USB
    iUSBProxy = new USBModedProxy(this);
    if (!iUSBProxy->isValid())
    {
        LOG_CRITICAL("Failed to connect to USB moded D-Bus interface");
        delete iUSBProxy;
        iUSBProxy = NULL;
    }
    else
    {
        QObject::connect(iUSBProxy, SIGNAL(usbConnection(bool)), this,
            SLOT(onUsbStateChanged(bool)));
        iTransportStates[Sync::CONNECTIVITY_USB] =
            iUSBProxy->isUSBConnected();
    }
#endif

    // BT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

    // Set the bluetooth state
    iTransportStates[Sync::CONNECTIVITY_BT] = btConnectivityStatus();

    // Add signal to track the bluetooth state changes
    iSystemBus = new QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus,
                                                                    QStringLiteral("buteo_system_bus")));
    if (!iSystemBus->connect("org.bluez",
                     "",
                     "org.bluez.Adapter",
                     "PropertyChanged",
                     this,
                     SLOT(onBtStateChanged(QString, QDBusVariant)
                     )))
    {
        LOG_WARNING("Unable to connect to system bus for org.bluez.Adapter");
    }

#else
    iTransportStates[Sync::CONNECTIVITY_BT] = iDeviceInfo.currentBluetoothPowerState();
    QObject::connect(&iDeviceInfo, SIGNAL(bluetoothStateChanged(bool)), this, SLOT(onBtStateChanged(bool)));
    LOG_DEBUG("Current bluetooth power state"<<iDeviceInfo.currentBluetoothPowerState());
#endif

    // Internet
    // @todo: enable when internet state is reported correctly.
    iInternet = new NetworkManager(this);
    if (iInternet != 0)
    {
        iTransportStates[Sync::CONNECTIVITY_INTERNET] =
            iInternet->isOnline();
        connect(iInternet,
                SIGNAL(statusChanged(bool, Sync::InternetConnectionType)),
                SLOT(onInternetStateChanged(bool, Sync::InternetConnectionType)) /*, Qt::QueuedConnection*/);
    }
    else
    {
        LOG_WARNING("Failed to listen for Internet state changes");
    }
}

TransportTracker::~TransportTracker()
{
    FUNCTION_CALL_TRACE;
    delete iSystemBus;
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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
void TransportTracker::onBtStateChanged(bool aState)
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aState);
    bool btPowered = iDeviceInfo.currentBluetoothPowerState();
    LOG_DEBUG("BT power state" << btPowered);
    updateState(Sync::CONNECTIVITY_BT, btPowered);
}
#else
void TransportTracker::onBtStateChanged(QString aKey, QDBusVariant aValue)
{
    FUNCTION_CALL_TRACE;

    if (aKey == "Powered")
    {
        bool btPowered = aValue.variant().toBool();
        LOG_DEBUG("BT power state " << btPowered);
        updateState(Sync::CONNECTIVITY_BT, btPowered);
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

void TransportTracker::updateState(Sync::ConnectivityType aType,
                                   bool aState)
{
    FUNCTION_CALL_TRACE;


    bool oldState = false;
    {
        QMutexLocker locker(&iMutex);
        oldState = iTransportStates[aType];
        iTransportStates[aType] = aState;
    }
    if(oldState != aState)
    {
        if (aType != Sync::CONNECTIVITY_INTERNET)
        {
            emit connectivityStateChanged(aType, aState);
        }
    }
}

bool TransportTracker::btConnectivityStatus()
{
    FUNCTION_CALL_TRACE;

    bool btOn = false;
    QDBusConnection *systemBus = new QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus,
                                                                                    QStringLiteral("buteo_system_bus2")));
    QDBusMessage methodCallMsg = QDBusMessage::createMethodCall("org.bluez",
                                                                "/",
                                                                "org.bluez.Manager",
                                                                "DefaultAdapter");

    QDBusMessage reply = systemBus->call(methodCallMsg);
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        LOG_WARNING("This device does not have a BT adapter");
        delete systemBus;
        return btOn;
    }

    QList<QVariant> adapterList = reply.arguments();
    // We will take the first adapter in the list
    QString adapterPath = qdbus_cast<QDBusObjectPath>(adapterList.at(0)).path();

    if (!adapterPath.isEmpty() || !adapterPath.isNull())
    {
        // Retrive the properties of the adapter and check for "Powered" key
        methodCallMsg = QDBusMessage::createMethodCall("org.bluez",
                                                       adapterPath,
                                                       "org.bluez.Adapter",
                                                       "GetProperties");
        reply = systemBus->call(methodCallMsg);
        if (reply.type() == QDBusMessage::ErrorMessage)
        {
            LOG_WARNING("Error in retrieving bluetooth properties");
            delete systemBus;
            return btOn;
        }

        QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
        if (arg.currentType() == QDBusArgument::MapType)
        {
            // Scan through the dict returned and check for "Powered" entry
            QMap<QString,QVariant> dict = qdbus_cast<QMap<QString,QVariant> >(arg);
            QMap<QString,QVariant>::iterator iter;
            for(iter = dict.begin(); iter != dict.end(); ++iter)
            {
                if (iter.key() == "Powered")
                {
                    btOn = iter.value().toBool();
                    LOG_DEBUG ("Bluetooth powered on? " << btOn);
                    break;
                }
            }
        }
    }
    delete systemBus;
    return btOn;
}
