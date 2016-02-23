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
#include "ClientThread.h"
#include "ClientPlugin.h"
#include "LogMacros.h"
#include <QCoreApplication>

using namespace Buteo;

ClientThread::ClientThread()
 : iClientPlugin( 0 ),
   iIdentity(NULL),
   iService(NULL),
   iSession(NULL),
   iRunning(false)
{
    FUNCTION_CALL_TRACE;
}

ClientThread::~ClientThread()
{
    FUNCTION_CALL_TRACE;
    if (iSession) {
        iIdentity->destroySession(iSession);
    }
    delete iIdentity;
}

QString ClientThread::getProfileName() const
{
    FUNCTION_CALL_TRACE;

    QString profileName;
    if (iClientPlugin != 0)
    {
        profileName = iClientPlugin->getProfileName();
    }

    return profileName;
}

ClientPlugin* ClientThread::getPlugin() const
{
    FUNCTION_CALL_TRACE;

    return iClientPlugin;
}

bool ClientThread::startThread( ClientPlugin* aClientPlugin )
{
    FUNCTION_CALL_TRACE;

    if (aClientPlugin == 0)
        return false;

    {
        QMutexLocker locker(&iMutex);
        if( iRunning ) {
            return false;
        }
        else
        {
            iRunning = true;
        }
    }

    iClientPlugin = aClientPlugin;
    if (iClientPlugin == 0)
    {
        LOG_CRITICAL("Client plugin is NULL");
        return false;
    }

    SyncProfile &profile = iClientPlugin->profile();
    const QString prefix("sso-provider=");
    QString username = profile.key("Username");
    if (username.startsWith(prefix)) {
        // Look up real username/password in SSO first,
        // before starting sync. This is better done
        // in the application thread, because this is where
        // this instance lives.
        iProvider = username.mid(prefix.size());
        LOG_DEBUG("SSO provider::" << iProvider);
        iService = new SignOn::AuthService(this);
        connect(iService, SIGNAL(identities(const QList<SignOn::IdentityInfo> &)),
                this, SLOT(identities(const QList<SignOn::IdentityInfo> &)));
        iService->queryIdentities();
    } else {
        // Move to client thread
        iClientPlugin->moveToThread( this );
        start();
    }

    return true;
}

void ClientThread::stopThread()
{
    FUNCTION_CALL_TRACE;
    exit();
}

void ClientThread::run()
{
    FUNCTION_CALL_TRACE;

    if( !iClientPlugin->init() ) {
        LOG_WARNING( "Could not initialize client plugin:" << iClientPlugin->getPluginName() );
        emit initError( getProfileName(), "", 0 );
        return;
    }
    
    if( !iClientPlugin->startSync() ) {
        LOG_WARNING( "Could not start client plugin:" << iClientPlugin->getPluginName() );
        emit initError( getProfileName(), "", 0 );
        return;
    }

    exec();

    iSyncResults = iClientPlugin->getSyncResults();

    iClientPlugin->uninit();

    // Move back to application thread
    iClientPlugin->moveToThread( QCoreApplication::instance()->thread() );
    
    {
        QMutexLocker locker(&iMutex);
        iRunning = false;
    }

}

SyncResults ClientThread::getSyncResults()
{
    FUNCTION_CALL_TRACE;

	return iSyncResults;
}

void ClientThread::identities(const QList<SignOn::IdentityInfo> &identityList)
{
    FUNCTION_CALL_TRACE;

    for (int i = 0; i < identityList.size(); ++i) {
        const SignOn::IdentityInfo &info = identityList.at(i);
        LOG_DEBUG("Signon identity::" << info.caption());
        if (info.caption() == iProvider) {
            iIdentity = SignOn::Identity::existingIdentity(info.id(), this);
            // Setup an authentication session using the "password" method
            iSession =
                iIdentity->createSession(QLatin1String("password"));
            connect(iSession, SIGNAL(response(const SignOn::SessionData &)),
                    this, SLOT(identityResponse(const SignOn::SessionData &)));
            connect(iSession, SIGNAL(error(SignOn::Error)),
                    this, SLOT(identityError(SignOn::Error)));
            // Get the password!
            iSession->process(SignOn::SessionData(), QLatin1String("password"));
            return;
        }
    }
    emit initError( getProfileName(), "credentials not found in SSO", 0 );
}

void ClientThread::identityResponse(const SignOn::SessionData &sessionData)
{
    FUNCTION_CALL_TRACE;

    // temporarily set real username/password, then invoke client
    SyncProfile &profile = iClientPlugin->profile();
    LOG_DEBUG("Username::" << sessionData.UserName());
    profile.setKey("Username", sessionData.UserName());
    profile.setKey("Password", sessionData.Secret());

    // delayed starting of thread
    iClientPlugin->moveToThread( this );
    start();
}

void ClientThread::identityError(SignOn::Error err)
{
    FUNCTION_CALL_TRACE;

    emit initError( getProfileName(), err.message(), 0 );
}

