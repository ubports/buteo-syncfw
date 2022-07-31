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

#ifndef SYNCBACKUP_H
#define SYNCBACKUP_H

#include "SyncBackupProxy.h"
#include "SyncBackupAdaptor.h"

namespace Buteo {

/*! \brief Handles Sync requirements towards Backup
 *
 * This class communicates with the Backup daemon for Backup state before
 * starting a sync.
 */
class SyncBackup : public SyncBackupProxy // Derived from QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Default Constructor
     */
    SyncBackup ();

    /*!
     * \brief Destructor
     */
    ~SyncBackup ();

    //! \brief  path to the backup dbus object
    static const char *DBUS_BACKUP_OBJECT;

    /*!
     * \brief Requests the current state og backup/restore operation.
     */
    bool getBackUpRestoreState();

    /*! \brief  Reply to backup framework, result of o/p
    * @param aResult - result
    */
    void sendReply (uchar aResult);

signals:

public slots:
    //  From backup framework ...

    /*! \brief Called by backup framework when backup starts
    * @param message - result
    * @return uchar
    */
    uchar backupStarts(const QDBusMessage &message);

    /*! Called by backup framework when backup is completed
    * @param message - result
    * @return uchar
    */
    uchar backupFinished(const QDBusMessage &message);

    /*! Called by backup framework when it starts to restore a backup.
    * @param message - result
    * @return uchar
    */
    uchar restoreStarts(const QDBusMessage &message);

    /*! Called by backup framework when backup ie restored
    * @param message - result
    * @return uchar
    */
    uchar restoreFinished(const QDBusMessage &message);

    /*! Called if backup service exits/aborts...
    * @param serviceName - name of the service
    */
    void backupServiceUnregistered(const QString  &serviceName);

private :
    bool iBackupRestore;
    QDBusMessage *iReply;
    QDBusServiceWatcher *iWatchService;
    SyncBackupAdaptor *iAdaptor;
    // Reply to backup dbus framework that response will
    // be delayed
    uchar sendDelayReply (const QDBusMessage &message);

#ifdef SYNCFW_UNIT_TESTS
    friend class SyncBackupTest;
#endif

};

}

#endif // SYNCBACKUP_H
