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

#ifndef CLIENTPLUGINRUNNER_H
#define CLIENTPLUGINRUNNER_H

#include "PluginRunner.h"
#include <QProcess>

namespace Buteo {

class ClientPlugin;
class ClientThread;
class SyncProfile;
    
/*! \brief Class for running client sync plug-ins
 */
class ClientPluginRunner : public PluginRunner
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * @param aPluginName Name of the plug-in to run
     * @param aProfile Sync profile for the client plug-in. Ownership is NOT
     *  transferred.
     * @param aPluginMgr PluginManager instance for creating and destroying
     *  plug-ins by name
     * @param aPluginCbIf Callback interface that the created plug-in can use
     * @param aParent Parent object
     */
    ClientPluginRunner(const QString &aPluginName, SyncProfile *aProfile,
        PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf,
        QObject *aParent = 0);

    //! \brief Destructor
    virtual ~ClientPluginRunner();

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

private slots:

    // Slots for catching plug-in signals.

    void onTransferProgress(const QString &aProfileName,
        Sync::TransferDatabase aDatabase, Sync::TransferType aType,
        const QString &aMimeType, int aCommittedItems);

    void onError(const QString &aProfileName, const QString &aMessage, int aErrorCode);

    void onSuccess(const QString &aProfileName, const QString &aMessage);

    void onStorageAccquired(const QString &aMimeType);

    void onSyncProgressDetail(const QString &aProfileName,int aProgressDetail);

    // Slot for observing thread exit
    void onThreadExit();

    void onProcessError( QProcess::ProcessError error );

    void onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void pluginTimeout();

private:

    SyncProfile *iProfile;

    ClientPlugin *iPlugin;

    ClientThread *iThread;
    
#ifdef SYNCFW_UNIT_TESTS
    friend class ClientPluginRunnerTest;
#endif

};

}

#endif // CLIENTPLUGINRUNNER_H
