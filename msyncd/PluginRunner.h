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


#ifndef PLUGINRUNNER_H
#define PLUGINRUNNER_H

#include <SyncResults.h>
#include "SyncCommonDefs.h"
#include "SyncPluginBase.h"
#include <QObject>
#include <QString>

namespace Buteo {

class PluginManager;
class PluginCbInterface;
    
/*! \brief Base class for running sync plug-ins.
 *
 * This class hides the details of thread/process handling when sync client
 * and server plug-ins are run. Specific client and server plug-in runner
 * classes are derived from this class.
 */
class PluginRunner : public QObject
{
    Q_OBJECT

public:

    //! Plug-in type: client or server
    enum PluginType {
        PLUGIN_CLIENT,
        PLUGIN_SERVER
    };

    /*! \brief Constructor
     *
     * @param aPluginType Type of the plug-in to run
     * @param aPluginName Name of the plug-in to run
     * @param aPluginMgr PluginManager instance for creating and destroying
     *  plug-ins by name
     * @param aPluginCbIf Callback interface that the created plug-in can use
     * @param aParent Parent object
     */
    PluginRunner(PluginType aPluginType, const QString &aPluginName,
        PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf,
        QObject *aParent = 0);

    /*! \brief Initializes the plug-in runner
     *
     * Creates the plug-in that will be run and a thread or process
     * for running it.
     * @return Success indicator
     */
    virtual bool init() = 0;

    /*! \brief Starts running the plug-in
     *
     * @return Success indicator.
     */
    virtual bool start() = 0;

    /*! \brief Stops running the plug-in
     *
     * Returns when the plug-in is stopped.
     */
    virtual void stop() = 0;

    /*! \brief Aborts running the plug-in
     *
     * The plug-in is requested to abort. This function will return when the
     * abort request is sent, but the plug-in will continue running until
     * it has gracefully aborted.
     */
    virtual void abort() = 0;

    /*! \brief Gets the sync results from the plug-in.
     *
     * Should be called only after success or error signal is received from
     * this class.
     * @return Sync results
     */
    virtual SyncResults syncResults() = 0;

    
    /*! \brief Calls the cleanup for the plugin  
     *
     * The plug-in is requested to clean up.
     */
    virtual bool cleanUp() = 0;

    /*! \brief Gets the plug-in type
     *
     * @return Plug-in type
     */
    PluginType pluginType() const;

    /*! \brief Gets the plug-in name
     *
     * @return Plug-in name
     */
    QString pluginName() const;

    /*! \brief Gets the plug-in associated with this plug-in runner
     *
     * @return Plug-in instance
     */
    virtual SyncPluginBase *plugin() = 0;

signals:
    //! @see SyncPluginBase::transferProgress
    void transferProgress(const QString &aProfileName,
        Sync::TransferDatabase aDatabase, Sync::TransferType aType,
        const QString &aMimeType);

    //! @see SyncPluginBase::error
    void error(const QString &aProfileName, const QString &aMessage, int aErrorCode);

    //! @see SyncPluginBase::success
    void success(const QString &aProfileName, const QString &aMessage);

    void storageAccquired (const QString &aMimeType);
    /*! \brief Signal sent when the plug-in runner has finished
     *
     * Sent when the thread or process running the plug-in has exited
     */
    void done();

    //! @see SyncPluginBase::newSession
    void newSession(const QString &aDestination);

    //! @see SyncPluginBase::connectivityStateChanged
    void connectivityStateChanged(Sync::ConnectivityType aType, bool aState);

protected:
    bool iInitialized;

    PluginManager *iPluginMgr;

    PluginCbInterface *iPluginCbIf;

    PluginType iType;

    QString iPluginName;

private:

	friend class PluginRunnerTest;
};

}

#endif // PLUGINRUNNER_H
