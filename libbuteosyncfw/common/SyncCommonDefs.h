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

#ifndef SYNCCOMMONDEFS_H
#define SYNCCOMMONDEFS_H

#include <QMetaType>
#include <QDir>

namespace Sync {

static const QString syncCacheDir()
{
    const QString HOME_PATH = (::getenv("XDG_CACHE_HOME") == NULL) ? QDir::homePath() + QDir::separator() + ".cache" : ::getenv("XDG_CACHE_HOME");
    return HOME_PATH + QDir::separator() + "msyncd";
}

enum SyncStatus {
    SYNC_QUEUED = 0,
    SYNC_STARTED,
    SYNC_PROGRESS,
    SYNC_ERROR,
    SYNC_DONE,
    SYNC_ABORTED,
    SYNC_CANCELLED,
    SYNC_STOPPING,
    SYNC_NOTPOSSIBLE,
    SYNC_AUTHENTICATION_FAILURE,
    SYNC_DATABASE_FAILURE,
    SYNC_CONNECTION_ERROR,
    SYNC_SERVER_FAILURE,
    SYNC_BAD_REQUEST,
    SYNC_PLUGIN_ERROR,
    SYNC_PLUGIN_TIMEOUT
};

// UI needs to display a detailed Progress for the Current ongoing sync
enum SyncProgressDetail {
    SYNC_PROGRESS_INITIALISING = 201,
    SYNC_PROGRESS_SENDING_ITEMS ,
    SYNC_PROGRESS_RECEIVING_ITEMS,
    SYNC_PROGRESS_FINALISING
};

enum TransferDatabase {
    LOCAL_DATABASE = 0,
    REMOTE_DATABASE
};

enum TransferType {
    ITEM_ADDED = 0,
    ITEM_MODIFIED,
    ITEM_DELETED,
    ITEM_ERROR
};

enum ConnectivityType {
    CONNECTIVITY_USB,
    CONNECTIVITY_BT,
    CONNECTIVITY_INTERNET
};

} // namespace Sync

Q_DECLARE_METATYPE( Sync::SyncStatus );
Q_DECLARE_METATYPE( Sync::TransferDatabase );
Q_DECLARE_METATYPE( Sync::TransferType );
Q_DECLARE_METATYPE( Sync::ConnectivityType );


#endif // SYNCCOMMONDEFS_H
