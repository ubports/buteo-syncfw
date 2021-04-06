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
#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QThread>
#include <QMutex>
#include <SyncResults.h>

#include "SignOn/AuthService"
#include "SignOn/Identity"

namespace Buteo {

class ClientPlugin;

/*! \brief Thread for client plugins
 *
 */
class ClientThread : public QThread
{
    Q_OBJECT
public:

    /*! \brief Constructor
     *
     */
    ClientThread();

    /*! \brief Destructor
     *
     */
    virtual ~ClientThread();

    /*! \brief Returns profile that this thread is running
     *
     * @return Profile name
     */
    QString getProfileName() const;

    /*! \brief Returns plugin that this thread is running
     *
     * @return Plugin
     */
    ClientPlugin *getPlugin() const;

    /*! \brief Starts client thread
     *
     * @param aClientPlugin Client plug-in to run. Plug-in is owned by the caller,
     *  and must not be deleted while the thread is running.
     * @return True on success, otherwise false
     */
    bool startThread(ClientPlugin *aClientPlugin);

    /*! \brief Stops client thread
     *
     */
    void stopThread();

    /*! \brief Returns the results for this particular thread
     *
     */
    SyncResults getSyncResults();

signals:

    /*! \brief Emitted when synchronization cannot be started due to an
     *         error in plugin initialization
     *
     * @param aProfileName Name of the profile being synchronized
     * @param aMessage Message data related to error event
     * @param aErrorCode Error code
     */
    void initError(const QString &aProfileName, const QString &aMessage, int aErrorCode);

protected:
    /*! \brief overriding method for QThread::run
     */
    virtual void run();

private:
    ClientPlugin   *iClientPlugin;

    SyncResults iSyncResults;

    SignOn::Identity *iIdentity;
    SignOn::AuthService *iService;
    SignOn::AuthSession *iSession;
    QString iProvider;

    bool iRunning;

    mutable QMutex iMutex;

#ifdef SYNCFW_UNIT_TESTS
    friend class ClientThreadTest;
#endif

    /*!
     * \brief invokes iClientPlugin->startSync()
     *
     * It should be called when profile is ready for use, with
     * credentials set in the Username/Password keys.  It is called
     * either in run() or, if the Username key starts with the
     * "sso-provider=" prefix, after retrieving the credentials from
     * SSO (queryIdentities() -> identities() -> session ->
     * identityResponse() -> startSync()).
     *
     * @return true for success (run thread), else failure (running
     * thread is no longer necessary)
     */
    bool startSync();

private slots:
    void identities(const QList<SignOn::IdentityInfo> &identityList);
    void identityResponse(const SignOn::SessionData &session);
    void identityError(SignOn::Error error);
};

}

#endif  //  CLIENTTHREAD_H
