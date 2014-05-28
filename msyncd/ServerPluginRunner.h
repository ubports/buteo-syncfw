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

#ifndef SERVERPLUGINRUNNER_H
#define SERVERPLUGINRUNNER_H

#include "PluginRunner.h"

namespace Buteo {
    

class ServerActivator;
class ServerPlugin;
class ServerThread;
class Profile;

/*! \brief Class for running server sync plug-ins
 */
class ServerPluginRunner : public PluginRunner
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * @param aPluginName Name of the plug-in to run
     * @param aProfile Profile for the server plug-in. Ownership is transferred.
     * @param aPluginMgr PluginManager instance for creating and destroying
     *  plug-ins by name
     * @param aPluginCbIf Callback interface that the created plug-in can use
     * @param aServerActivator Server activator, controls enabled/disabled state
     *  of the server plug-in
     * @param aParent Parent object
     */
    ServerPluginRunner(const QString &aPluginName, Profile *aProfile,
        PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf,
        ServerActivator *aServerActivator, QObject *aParent = 0);

    //! \brief Destructor
    virtual ~ServerPluginRunner();

    //! @see PluginRunner::init
    virtual bool init();

    //! @see PluginRunner::start
    virtual bool start();

    //! @see PluginRunner::stop
    virtual void stop();

    //! @see PluginRunner::abort
    virtual void abort(Sync::SyncStatus aStatus = Sync::SYNC_ABORTED);

    //! @see PluginRunner::syncResults
    virtual SyncResults syncResults();

    //! @see PluginRunner::plugin
    virtual SyncPluginBase *plugin();
    
    //! @see PluginRunner::plugin
    virtual bool cleanUp();

    // Suspend a server plug-in
    void suspend();

    // Resume a suspended server plug-in
    void resume();

private slots:

    // Slots for catching plug-in signals.

    void onNewSession(const QString &aDestination);

    void onTransferProgress(const QString &aProfileName,
        Sync::TransferDatabase aDatabase, Sync::TransferType aType,
        const QString &aMimeType, int aCommittedItems);

    void onStorageAccquired(const QString &aMimeType );
    void onError(const QString &aProfileName, const QString &aMessage, int aErrorCode);

    void onSuccess(const QString &aProfileName, const QString &aMessage);

    // Slot for observing thread exit
    void onThreadExit();

    void onProcessError( QProcess::ProcessError error );

    void onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

private:

    void onSessionDone();

    Profile *iProfile;

    ServerPlugin *iPlugin;

    ServerThread *iThread;

    ServerActivator *iServerActivator;

#ifdef SYNCFW_UNIT_TESTS
    friend class ServerPluginRunnerTest;
#endif

};

}

#endif // SERVERPLUGINRUNNER_H
