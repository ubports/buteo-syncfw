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
   iRunning(false)
{
    FUNCTION_CALL_TRACE;
}

ClientThread::~ClientThread()
{
    FUNCTION_CALL_TRACE;
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

    // Move to client thread
    iClientPlugin->moveToThread( this );

    start();

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

    if (iClientPlugin == 0)
    {
        LOG_CRITICAL("Client plugin is NULL");
        return;
    }

    if( !iClientPlugin->init() ) {
        LOG_DEBUG( "Could not initialize client plugin:" << iClientPlugin->getPluginName() );
        emit initError( getProfileName(), "", 0 );
        return;
    }

    if( !iClientPlugin->startSync() ) {
        LOG_DEBUG( "Could not start client plugin:" << iClientPlugin->getPluginName() );
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

