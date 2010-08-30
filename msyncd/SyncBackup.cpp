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

#include "SyncBackup.h"
#include "SyncBackupAdaptor.h"
#include "LogMacros.h"
#include "SyncDBusConnection.h"

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusServiceWatcher>

using namespace Buteo;

const char* SyncBackup::DBUS_BACKUP_OBJECT = "/backup";
const QString BACKUP_SERVICE_NAME = "com.nokia.backup";

SyncBackup::SyncBackup() :
	iBackupRestore(false),
	iReply(0),
        iWatchService(0),
        iAdaptor(0) 
{
    FUNCTION_CALL_TRACE;
   
    iAdaptor = new SyncBackupAdaptor(this);
    if (!iAdaptor) {
    	LOG_CRITICAL("Failed to initialize adaptor");
	Q_ASSERT(false);
    }

    QDBusConnection dbus = SyncDBusConnection::sessionBus();

    if (dbus.registerObject(DBUS_BACKUP_OBJECT, this)) {
	    LOG_DEBUG("Registered sync backup to D-Bus");
    } else {
	    LOG_CRITICAL("Failed to register sync backup to D-Bus");
            Q_ASSERT(false); 
    }
    
    iWatchService = new QDBusServiceWatcher (BACKUP_SERVICE_NAME , dbus, QDBusServiceWatcher::WatchForUnregistration);
    if (!iWatchService) {
    	LOG_CRITICAL("Failed to initialize watch service : backup");
	Q_ASSERT(false);
    }

    connect(iWatchService, SIGNAL(serviceUnregistered(const QString&)),
		    this, SLOT(backupServiceUnregistered(const QString&)));
}

SyncBackup::~SyncBackup()
{
    FUNCTION_CALL_TRACE;
    iBackupRestore = false;
    //Unregister from D-Bus.
    QDBusConnection dbus = SyncDBusConnection::sessionBus();
    dbus.unregisterObject(DBUS_BACKUP_OBJECT);
    delete iWatchService;
    iWatchService = 0;
    delete iAdaptor;
    iAdaptor = 0;
    LOG_DEBUG("Unregistered backup from D-Bus");
}

void SyncBackup::backupServiceUnregistered(const QString  &serviceName) 
{
	FUNCTION_CALL_TRACE;
	Q_UNUSED (serviceName);
	if (iBackupRestore) {
	// Should not happen ; backup framework exitted abruptly
		emit restoreDone();
	}
	iBackupRestore = false;
}

uchar SyncBackup::sendDelayReply (const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE;
#ifdef UNIT_TEST
         return 0;
#endif 
    message.setDelayedReply(true);
    if (!iReply)
	    iReply = new QDBusMessage;
    *iReply = message.createReply();
    return 0;
}

void SyncBackup::sendReply (uchar aResult)
{
    FUNCTION_CALL_TRACE;
#ifdef UNIT_TEST
         return ;
#endif 
    if (iReply) {
	    LOG_DEBUG ("Send Reply");
	    QList<QVariant>  arguments;
	    QVariant vt = QVariant::fromValue((uchar)aResult);
            arguments.append(vt);
            iReply->setArguments(arguments);
	    SyncDBusConnection::sessionBus().send(*iReply);
	    delete iReply;
	    iReply = 0;
    }
}

uchar SyncBackup::backupStarts(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE;
    iBackupRestore = true;
    sendDelayReply(message);
    emit startBackup();
    return 0;
}

uchar SyncBackup::backupFinished(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (message);
    iBackupRestore = false;
    sendDelayReply(message);
    emit backupDone();
    return 0;
}

uchar SyncBackup::restoreStarts(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE;
    iBackupRestore = true;
    sendDelayReply(message);
    emit startRestore();
    return 0;
}

uchar SyncBackup::restoreFinished(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (message);
    iBackupRestore = false;
    sendDelayReply(message);
    emit restoreDone();
    return 0;
}
    
bool SyncBackup::getBackUpRestoreState()
{
    FUNCTION_CALL_TRACE;
    return iBackupRestore;    
}
