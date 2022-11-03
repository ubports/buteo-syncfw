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
#ifndef NETWORKMANAGER_H_
#define NETWORKMANAGER_H_

#include <QNetworkSession>
#include <QTimer>
#include "SyncCommonDefs.h"

class QNetworkConfigurationManager;

namespace Buteo
{

    /*! \brief Class for managing network sessions
    *
    * This class provides APIs to open and close network sessions. It internally
    * uses QNetworkSession set of classes to manage network sessions. The user
    * while creating a new session can choose whether to pop-up the internet
    * connectivity dialog or not. The class also signals about online status of
    * the device.
    */
    class NetworkManager : public QObject
    {
        Q_OBJECT
        public:
            /*! \brief Constructor
             *
             * @param parent Parent object
             */
            NetworkManager(QObject *parent = 0);

            /*! \brief Destructor
             *
             */
            ~NetworkManager();

            /*! \brief Returns if the device is currently online, i.e, a data
             * sessions is possible.
             *
             * @return True if device is online, false if not
             */
            bool isOnline();

            /*! \brief Returns the type of connection used by the device.
             *

             * @return Sync::InternetConnectionType the type of connection.
             */
            Sync::InternetConnectionType connectionType() const;

            /*! \brief Connects a new network session. If a session was already
             * open, the signal connectionSuccess will be emitted immediately,
             * else the function will return and the signal connectionSuccess or
             * connectionError will be emitted accordingly. The caller can
             * choose whether to show the network connectivity dialog, or just
             * open the default network configuration in the background using
             * the parameter connetInBackground.
             *
             * @param connectInBackground If true, the connection is opened
             * without popping up the connetion dialog
             */
            void connectSession(bool connectInBackground = false);

            /*! \brief Disconnects an open session
             *
             */
            void disconnectSession();
signals:
            /*! \brief This signal is emitted when the device's online status
             * changes
             *
             * @param aConnected If true, the device is online
             */
            void statusChanged(bool aConnected, Sync::InternetConnectionType aType);

            /*! \brief This signal is emitted when a network session gets
             * connected
             *
             */
            void connectionSuccess();

            /*! \brief This signal is emitted when opening a network session
             * fails
             *
             */
            void connectionError();
        private:
            QNetworkConfigurationManager    *m_networkConfigManager;    // QT network configuration manager
            QNetworkSession                 *m_networkSession;          // QT network session
            static bool                     m_isSessionActive;          // Flag to indicate if a network session is active
            bool                            m_isOnline;                 // Flag to indicate if the device is online
            static int                      m_refCount;                 // Reference counter for number of open connections
            bool                            m_errorEmitted;             // Network error emited flag
            QTimer                          *m_sessionTimer;
            Sync::InternetConnectionType    m_connectionType;
            QTimer                          m_idleRefreshTimer;

private slots:
            void slotSessionState(QNetworkSession::State status);
            void slotSessionError(QNetworkSession::SessionError error);
            void sessionConnectionTimeout();
            void slotConfigurationChanged();
            void idleRefresh();
    };
}

#endif//NETWORKMANAGER_H_

