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

#ifndef SYNCBACKUPPROXY_H
#define SYNCBACKUPPROXY_H

#include <QtDBus>
#include <QObject>
#include <QString>

namespace Buteo {

/*!
 * \brief Defines a D-Bus backup proxy for the backupclient
 *
 * A XML file describing the interface can be generated from this class using
 * qdbuscpp2xml tool. This XML file can then be used to generate interface
 * adaptor and proxy classes using qdbusxml2cpp tool.
 */
class SyncBackupProxy : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.backupclient")

public:

signals:

    
    /*! \brief Notifies about completion of backup.
     *
     * This signal is sent when the backup is completed 
     */
    void backupDone();
    
    /*! \brief Notifies about starting of backup.
     *
     * This signal is sent when the backup is started
     */
    void startBackup();
    
    /*! \brief Notifies about completion of restore opertaion.
     *
     * This signal is sent when the backup is completed 
     */
    void restoreDone();
    
    /*! \brief Notifies about starting of restore operation.
     *
     * This signal is sent when the restore is started/in progress 
     */
    void startRestore();

public slots:

    /*!
     * \brief Sets the required params and stops the servers and any running sync
     * sessions.
     *
     * This function must be called when backup is initiated,
     * @param message  Received dbus message
     */
    virtual uchar  backupStarts (const QDBusMessage &message) = 0;
    
    /*!
     * \brief Sets the required params and starts the servers.
     *
     * This function must be called when backup is completed.
     * @param message  Received dbus message
     */
    virtual uchar backupFinished (const QDBusMessage &message) = 0;
    
    /*!
     * \brief Sets the required params and stops the servers and any running sync
     * sessions.
     *
     * This function must be called when restore is initiated,
     * @param message  Received dbus message
     */
    virtual uchar restoreStarts (const QDBusMessage &message) = 0;
    
    /*!
     * \brief Sets the required params and starts the servers.
     *
     * This function must be called when restore is completed.
     * @param message  Received dbus message
     */
    virtual uchar restoreFinished (const QDBusMessage &message) = 0;
    
    /*!
     * \brief Requests the current state og backup/restore operation.
     */
    virtual bool getBackUpRestoreState() = 0;

};
}
#endif // SYNCBACKUPPROXY_H
