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
#include <QNetworkSession>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QTimer>

#include "NetworkManager.h"
#include "LogMacros.h"

namespace {
    Sync::InternetConnectionType convertNetworkConnectionType(QNetworkConfiguration::BearerType connectionType)
    {
        switch (connectionType) {
            case QNetworkConfiguration::BearerEthernet:  return Sync::INTERNET_CONNECTION_ETHERNET;
            case QNetworkConfiguration::BearerWLAN:      return Sync::INTERNET_CONNECTION_WLAN;
            case QNetworkConfiguration::Bearer2G:        return Sync::INTERNET_CONNECTION_2G;
            case QNetworkConfiguration::BearerCDMA2000:  return Sync::INTERNET_CONNECTION_CDMA2000;
            case QNetworkConfiguration::BearerWCDMA:     return Sync::INTERNET_CONNECTION_WCDMA;
            case QNetworkConfiguration::BearerHSPA:      return Sync::INTERNET_CONNECTION_HSPA;
            case QNetworkConfiguration::BearerBluetooth: return Sync::INTERNET_CONNECTION_BLUETOOTH;
            case QNetworkConfiguration::BearerWiMAX:     return Sync::INTERNET_CONNECTION_WIMAX;
            case QNetworkConfiguration::BearerEVDO:      return Sync::INTERNET_CONNECTION_EVDO;
            case QNetworkConfiguration::BearerLTE:       return Sync::INTERNET_CONNECTION_LTE;
            case QNetworkConfiguration::Bearer3G:        return Sync::INTERNET_CONNECTION_3G;
            case QNetworkConfiguration::Bearer4G:        return Sync::INTERNET_CONNECTION_4G;
            default:                                     return Sync::INTERNET_CONNECTION_UNKNOWN;
        }
    }
}

using namespace Buteo;

int NetworkManager::m_refCount = 0;
bool NetworkManager::m_isSessionActive = false;

NetworkManager::NetworkManager(QObject *parent /* = 0*/) :
    QObject(parent), m_networkConfigManager(0), m_networkSession(0),
    m_isOnline(false), m_errorEmitted(false),
    m_sessionTimer(0), m_connectionType(Sync::INTERNET_CONNECTION_UNKNOWN)
{
    FUNCTION_CALL_TRACE;
    m_networkConfigManager = new QNetworkConfigurationManager();
    Q_ASSERT(m_networkConfigManager);

    // check for network status and configuration change (switch wifi, ethernet, mobile) a
    connect(m_networkConfigManager,
            SIGNAL(onlineStateChanged(bool)),
            SLOT(slotConfigurationChanged()),
            Qt::QueuedConnection);
    connect(m_networkConfigManager,
            SIGNAL(configurationAdded(QNetworkConfiguration)),
            SLOT(slotConfigurationChanged()),
            Qt::QueuedConnection);
    connect(m_networkConfigManager,
            SIGNAL(configurationChanged(QNetworkConfiguration)),
            SLOT(slotConfigurationChanged()),
            Qt::QueuedConnection);
    connect(m_networkConfigManager,
            SIGNAL(configurationRemoved(QNetworkConfiguration)),
            SLOT(slotConfigurationChanged()),
            Qt::QueuedConnection);
    connect(m_networkConfigManager,
            SIGNAL(updateCompleted()),
            SLOT(slotConfigurationChanged()),
            Qt::QueuedConnection);

    connect(&m_idleRefreshTimer,
            SIGNAL(timeout()),
            SLOT(idleRefresh()),
            Qt::QueuedConnection);
    m_idleRefreshTimer.setSingleShot(true);

    // check connection status on startup
    idleRefresh();
    LOG_INFO("Network status:");
    LOG_INFO("\tOnline::" << m_isOnline);
    LOG_INFO("\tConnection::" << m_connectionType);

    m_sessionTimer = new QTimer(this);
    m_sessionTimer->setSingleShot(true);
    m_sessionTimer->setInterval(10000);
    connect(m_sessionTimer,SIGNAL(timeout()),this,SLOT(sessionConnectionTimeout()));
}

NetworkManager::~NetworkManager()
{
    FUNCTION_CALL_TRACE;
    if(m_networkSession)
    {
        delete m_networkSession;
        m_networkSession = 0;
    }
    if(m_networkConfigManager)
    {
        delete m_networkConfigManager;
        m_networkConfigManager = 0;
    }
}

bool NetworkManager::isOnline()
{
    FUNCTION_CALL_TRACE;
    return m_isOnline;
}

Sync::InternetConnectionType NetworkManager::connectionType() const
{
    return m_connectionType;
}

void NetworkManager::connectSession(bool connectInBackground /* = false*/)
{
    FUNCTION_CALL_TRACE;
    if(m_isSessionActive)
    {
        LOG_DEBUG("Network session already active, ignoring connect call");
        m_refCount++;
        emit connectionSuccess();
        return;
    }
    else if(!m_networkSession)
    {
        QNetworkConfiguration netConfig = m_networkConfigManager->defaultConfiguration();
        m_networkSession = new QNetworkSession(netConfig);
        m_errorEmitted = false;

        Q_ASSERT(m_networkSession);

        connect(m_networkSession, SIGNAL(error(QNetworkSession::SessionError)),
                SLOT(slotSessionError(QNetworkSession::SessionError)));
        connect(m_networkSession, SIGNAL(stateChanged(QNetworkSession::State)),
                SLOT(slotSessionState(QNetworkSession::State)));
        connect(m_networkSession, SIGNAL(opened()), SIGNAL(connectionSuccess()));
    }
    m_networkSession->setSessionProperty("ConnectInBackground", connectInBackground);
    if(!m_networkSession->isOpen()) {
        m_networkSession->open();
        // Fail after 10 sec if no network reply is received
        m_sessionTimer->start();
    } else {
        slotSessionState(m_networkSession->state());
    }
}

void NetworkManager::sessionConnectionTimeout()
{
    if (!m_errorEmitted && m_networkSession) {
        if (!m_networkSession->isOpen()) {
            LOG_WARNING("No network reply received after 10 seconds, emitting session error.");
            slotSessionError(m_networkSession->error());
        }
    }
}

void NetworkManager::slotConfigurationChanged()
{
    // wait for 3 secs before update connection status
    // this avoid problems with connections that take a while to be stabilished
    m_idleRefreshTimer.start(3000);
}

void NetworkManager::idleRefresh()
{
    FUNCTION_CALL_TRACE;
    QList<QNetworkConfiguration> activeConfigs = m_networkConfigManager->allConfigurations(QNetworkConfiguration::Active);
    QNetworkConfiguration::BearerType connectionType = QNetworkConfiguration::BearerUnknown;
    QString bearerTypeName;

    bool isOnline = activeConfigs.size() > 0;
    if (isOnline)
    {
        // FIXME: due this bug lp:#1444162 on nm the QNetworkConfigurationManager
        // returns the wrong default connection.
        // We will consider the connection with the smallest bearer as the
        // default connection, with that wifi and ethernet will be the first one
        // https://bugs.launchpad.net/ubuntu/+source/network-manager/+bug/1444162
        connectionType = activeConfigs.first().bearerType();
        bearerTypeName = activeConfigs.first().bearerTypeName();
        foreach(const QNetworkConfiguration &conf, activeConfigs)
        {
            if (conf.bearerType() < connectionType)
            {
                connectionType = conf.bearerType();
                bearerTypeName = conf.bearerTypeName();
            }
        }
    }

    const Sync::InternetConnectionType convertedConnectionType = convertNetworkConnectionType(connectionType);
    LOG_INFO("New network state:" << isOnline << " New type: " << bearerTypeName << "(" << convertedConnectionType << ")");
    if (isOnline != m_isOnline || convertedConnectionType != m_connectionType)
    {
        m_isOnline = isOnline;
        m_connectionType = convertedConnectionType;
        emit statusChanged(m_isOnline, m_connectionType);
    }
}

void NetworkManager::disconnectSession()
{
    FUNCTION_CALL_TRACE;
    if(m_refCount > 0)
    {
        m_refCount--;
    }
    if(m_networkSession && 0 == m_refCount)
    {
        if (m_sessionTimer->isActive())
            m_sessionTimer->stop();

        m_networkSession->close();
        delete m_networkSession;
        m_networkSession = NULL;
    }
}

void NetworkManager::slotSessionState(QNetworkSession::State status)
{
    FUNCTION_CALL_TRACE;
    switch(status)
    {
    case QNetworkSession::Invalid:
        LOG_WARNING("QNetworkSession::Invalid");
        m_isSessionActive = false;
        break;
    case QNetworkSession::NotAvailable:
        LOG_WARNING("QNetworkSession::NotAvailable");
        m_isSessionActive = false;
        emit connectionError();
        break;
    case QNetworkSession::Connecting:
        LOG_DEBUG("QNetworkSession::Connecting");
        m_isSessionActive = false;
        break;
    case QNetworkSession::Connected:
        LOG_WARNING("QNetworkSession::Connected");
        if (m_networkSession->isOpen() &&
                m_networkSession->state() == QNetworkSession::Connected)
        {
            m_isSessionActive = true;
            emit connectionSuccess();
        }
        else
        {
            emit connectionError();
        }
        break;
    case QNetworkSession::Closing:
        LOG_WARNING("QNetworkSession::Closing");
        m_isSessionActive = false;
        break;
    case QNetworkSession::Disconnected:
        LOG_DEBUG("QNetworkSession::Disconnected");
        m_isSessionActive = false;
        break;
    case QNetworkSession::Roaming:
        LOG_WARNING("QNetworkSession::Roaming");
        m_isSessionActive = false;
        break;
    default:
        LOG_WARNING("QNetworkSession:: Unknown status change");
        m_isSessionActive = false;
        break;
    }
}

void NetworkManager::slotSessionError(QNetworkSession::SessionError error)
{
    FUNCTION_CALL_TRACE;

    // Emit network errors only once per request
    if (m_errorEmitted) {
        m_errorEmitted = false;
        return;
    } else {
        m_errorEmitted = true;
    }

    switch(error)
    {
    case QNetworkSession::UnknownSessionError:
        LOG_WARNING("QNetworkSession::UnknownSessionError");
        emit connectionError();
        break;
    case QNetworkSession::SessionAbortedError:
        LOG_DEBUG("QNetworkSession::SessionAbortedError");
        emit connectionError();
        break;
    case QNetworkSession::RoamingError:
        LOG_WARNING("QNetworkSession::RoamingError");
        emit connectionError();
        break;
    case QNetworkSession::OperationNotSupportedError:
        LOG_WARNING("QNetworkSession::OperationNotSupportedError");
        emit connectionError();
        break;
    case QNetworkSession::InvalidConfigurationError:
        LOG_WARNING("QNetworkSession::InvalidConfigurationError");
        emit connectionError();
        break;
    default:
        LOG_WARNING("QNetworkSession:: Invalid error code");
        emit connectionError();
        break;
    }
}

