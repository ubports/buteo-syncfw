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


using namespace Buteo;

TransportTracker::TransportTracker(QObject *aParent) :
    QObject(aParent),
    iUSBProxy(0),
    iInternet(0)
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
#warning "Not listening to bluetooth power state changes on Qt5"
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
        connect(iInternet, SIGNAL(valueChanged(bool)),
            this, SLOT(onInternetStateChanged(bool)) /*, Qt::QueuedConnection*/);
    }
    else
    {
        LOG_WARNING("Failed to listen for Internet state changes");
    }
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

void TransportTracker::onBtStateChanged(bool aState)
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aState);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
/// @todo Qt5 version of systeminfo does not have bluetooth power state at the moment
#warning "Qt5 ports always reports bluetooth power state as false for now"
    bool btPowered = false;
#else
    bool btPowered = iDeviceInfo.currentBluetoothPowerState();
#endif
    LOG_DEBUG("BT power state" << btPowered);
    updateState(Sync::CONNECTIVITY_BT, btPowered);
}

void TransportTracker::onInternetStateChanged(bool aConnected)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Internet state changed:" << aConnected);
    updateState(Sync::CONNECTIVITY_INTERNET, aConnected);
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
        if (aType != Sync::CONNECTIVITY_INTERNET) {
            emit connectivityStateChanged(aType, aState);
        }
        else {
            emit networkStateChanged(aState);
        }
    }
}
