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
#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QMutex>

namespace Buteo {

class ServerPlugin;
    
/*! \brief Thread for server plugin
 *
 */
class ServerThread : public QThread
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     */
    ServerThread();

    /*! \brief Destructor
     *
     */
    virtual ~ServerThread();

    /*! \brief Returns profile that this thread is running
     *
     * @return Profile name
     */
    QString getProfileName() const;

    /*! \brief Returns plugin that this thread is running
     *
     * @return Plugin
     */
    ServerPlugin* getPlugin() const;

    /*! \brief Starts server thread
     *
     * @param aServerPlugin Server plug-in to run. The plug-in is owned by the caller
     *  and must not be deleted while the thread is running.
     * @return True on success, otherwise false
     */
    bool startThread( ServerPlugin* aServerPlugin );

    /*! \brief Stops server thread
     *
     */
    void stopThread();

signals:

        /*! \brief Emitted when synchronization cannot be started due to an
         *         error in plugin initialization
         *
         * @param aProfileName Name of the profile being synchronized
         * @param aMessage Message data related to error event
         * @param aErrorCode Error code
         */
    void initError( const QString &aProfileName, const QString &aMessage,
        int aErrorCode);

protected:

    //! overriding method of QThread::run
    virtual void run();

private:

    ServerPlugin *iServerPlugin;

    bool iRunning;

    mutable QMutex iMutex;

#ifdef SYNCFW_UNIT_TESTS
	friend class ServerThreadTest;
#endif

};

}

#endif  //  SERVERTHREAD_H
