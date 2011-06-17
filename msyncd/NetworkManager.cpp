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

#include "NetworkManager.h"
#include "LogMacros.h"

using namespace Buteo;

int NetworkManager::m_refCount = 0;
bool NetworkManager::m_isSessionActive = false;

NetworkManager::NetworkManager(QObject *parent /* = 0*/) : 
    QObject(parent), m_networkConfigManager(0), m_networkSession(0),
    m_isOnline(false)
{
    FUNCTION_CALL_TRACE;
    m_networkConfigManager = new QNetworkConfigurationManager();
    Q_ASSERT(m_networkConfigManager);
    connect(m_networkConfigManager, SIGNAL(onlineStateChanged(bool)),
                SLOT(slotOnlineStateChanged(bool)));
    m_isOnline = m_networkConfigManager->isOnline();
    LOG_DEBUG("Online status::" << m_isOnline);
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
#ifndef __ARMEL__
    return true;
#endif
    return m_isOnline;
}

void NetworkManager::connectSession(bool connectInBackground /* = false*/)
{
#ifndef __ARMEL__
    emit connectionSuccess();
    return;
#endif
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
        
        Q_ASSERT(m_networkSession);
        
        connect(m_networkSession, SIGNAL(error(QNetworkSession::SessionError)),
                SLOT(slotSessionError(QNetworkSession::SessionError)));
        connect(m_networkSession, SIGNAL(stateChanged(QNetworkSession::State)),
                SLOT(slotSessionState(QNetworkSession::State)));
        /*
        connect(m_networkSession, SIGNAL(opened()),
                SLOT(slotSessionOpened()));
                */
    }
    m_networkSession->setSessionProperty("ConnectInBackground", connectInBackground);
    if(!m_networkSession->isOpen())
        m_networkSession->open();
    else
	slotSessionState(m_networkSession->state());    
}

void NetworkManager::disconnectSession()
{
#ifndef __ARMEL__
    return;
#endif
    FUNCTION_CALL_TRACE;
    if(m_refCount > 0)
    {
        m_refCount--;
    }
    if(m_networkSession && 0 == m_refCount)
    {
        m_networkSession->close();
	delete m_networkSession;
	m_networkSession = NULL;
    }
}

void NetworkManager::slotOnlineStateChanged(bool isOnline)
{
#ifndef __ARMEL__
    return;
#endif
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Online status changed, is online is now::" << isOnline);
    if(m_isOnline != isOnline)
    {
        m_isOnline = isOnline;
        emit valueChanged(m_isOnline);
    }
}

void NetworkManager::slotSessionState(QNetworkSession::State status)
{
#ifndef __ARMEL__
    return;
#endif
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
#ifndef __ARMEL__
    return;
#endif
    FUNCTION_CALL_TRACE;
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

