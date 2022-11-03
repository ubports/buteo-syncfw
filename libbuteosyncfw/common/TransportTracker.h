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

#ifndef TRANSPORTTRACKER_H_
#define TRANSPORTTRACKER_H_

#include "SyncCommonDefs.h"
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QDBusVariant>
#include <QDBusConnection>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtSystemInfo/QSystemDeviceInfo>
#endif

namespace Buteo {

class USBModedProxy;
class NetworkManager;


/*! \brief Class for tracking transport states
 *
 * USB state is tracked with HAL, BT with Context Framework and Internet states with Buteo::NetworkManager.
 */
class TransportTracker : public QObject
{
        Q_OBJECT

public:

        /*! \brief Constructor
         *
         * @param aParent Parent object
         */
        TransportTracker(QObject *aParent = 0);

        //! \brief Destructor
        virtual ~TransportTracker();

        /*! \brief Checks the state of the given connectivity type
         *
         * @param aType Connectivity type
         * @return True if available, false if not
         */
        bool isConnectivityAvailable(Sync::ConnectivityType aType) const;

signals:

    /*! \brief Signal emitted when a connectivity state changes
     *
     * @param aType Connectivity type whose state has changed
     * @param aState New state. True if available, false if not.
     */
        void connectivityStateChanged(Sync::ConnectivityType aType, bool aState);

    /*! \brief Signal emitted when a n/w state changes
     *
     * @param aState New state. True if available, false if not.
     * @param aType Connection type. The type of connetcion with the Internet.
     */
    void networkStateChanged(bool aState, Sync::InternetConnectionType aType);

    /*! \brief Signal emitted when a network session is successfully opened
     */
    void sessionConnected();

    /*! \brief Signal emitted when opening a network session fails
     */
    void sessionError();

private slots:

        void onUsbStateChanged(bool aConnected);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    void onBtStateChanged(bool aState);
#else
    void onBtStateChanged(QString aKey, QDBusVariant aValue);
#endif

    void onInternetStateChanged(bool aConnected, Sync::InternetConnectionType aType);

private:

    QMap<Sync::ConnectivityType, bool> iTransportStates;

    USBModedProxy *iUSBProxy;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QtMobility::QSystemDeviceInfo iDeviceInfo;
#endif

    NetworkManager *iInternet;
    QDBusConnection *iSystemBus;

    mutable QMutex iMutex;

        /*! \brief updates the state of the given connectivity type to input value
         *
         * @param aType Connectivity type
         * @param aState Connectivity State
         */
        void updateState(Sync::ConnectivityType aType, bool aState);

#ifdef SYNCFW_UNIT_TESTS
    friend class TransportTrackerTest;
    friend class SynchronizerTest;
#endif

    bool btConnectivityStatus();

};

}

#endif /* TRANSPORTTRACKER_H_ */
