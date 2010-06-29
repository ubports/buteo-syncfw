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
//#include "USBModedProxy.h"
#include "LogMacros.h"
#include <contextsubscriber/contextproperty.h>
#include <QMutexLocker>


using namespace Buteo;

TransportTracker::TransportTracker(QObject *aParent) :
    QObject(aParent),
    iUSBProxy(0),
    iBt(0),
    iInternet(0)
{
    FUNCTION_CALL_TRACE;

    iTransportStates[Sync::CONNECTIVITY_USB] = false;
    iTransportStates[Sync::CONNECTIVITY_BT] = false;
    // @todo: initialize to false
    iTransportStates[Sync::CONNECTIVITY_INTERNET] = true; //false;

    // USB
//    iUSBProxy = new USBModedProxy(this);
//    if (!iUSBProxy->isValid())
//    {
//        LOG_CRITICAL("Failed to connect to USB moded D-Bus interface");
//        delete iUSBProxy;
//        iUSBProxy = NULL;
//    }
//    else
//    {
//        QObject::connect(iUSBProxy, SIGNAL(usbConnection(bool)), this,
//            SLOT(onUsbStateChanged(bool)));
//        iTransportStates[Sync::CONNECTIVITY_USB] =
//            iUSBProxy->isUSBConnected();
//    }

    // BT
    iBt = new ContextProperty("Bluetooth.Enabled", this);
    if (iBt != 0)
    {
        iTransportStates[Sync::CONNECTIVITY_BT] = iBt->value().toBool();
        connect(iBt, SIGNAL(valueChanged()), this, SLOT(onBtStateChanged()));
    }
    else
    {
        LOG_WARNING("Failed to listen for BT state changes");
    }

    // Internet
    // @todo: enable when internet state is reported correctly.
    //iInternet = new ContextProperty("Internet.NetworkState", this);
    if (iInternet != 0)
    {
        iTransportStates[Sync::CONNECTIVITY_INTERNET] =
            iInternet->value().toBool();
        connect(iInternet, SIGNAL(valueChanged()),
            this, SLOT(onInternetStateChanged()));
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

void TransportTracker::onBtStateChanged()
{
    FUNCTION_CALL_TRACE;

    bool newState = false;
    if (iBt != 0)
    {
        newState = iBt->value().toBool();
    }
    LOG_DEBUG("BT state changed:" << newState);
    updateState(Sync::CONNECTIVITY_BT, newState);
}

void TransportTracker::onInternetStateChanged()
{
    FUNCTION_CALL_TRACE;

    bool newState = false;
    if (iInternet != 0)
    {
        newState = iInternet->value().toBool();
    }
    LOG_DEBUG("Internet state changed:" << newState);
    updateState(Sync::CONNECTIVITY_INTERNET, newState);
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
    if (oldState != aState)
    {
        emit connectivityStateChanged(aType, aState);
    }
}

