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
#ifndef SERVERPLUGIN_H_4399340943904399349843
#define SERVERPLUGIN_H_4399340943904399349843

#include "SyncPluginBase.h"
#include "libsyncprofile/Profile.h"
#include <QObject>
#include <QString>

namespace Buteo {
    
/*! \brief Base class for server plugins
 *
 */
class ServerPlugin : public SyncPluginBase
{
    Q_OBJECT;

public:

    /*! \brief Constructor
     *
     * @param aPluginName Name of this plugin
     * @param aProfile Server profile
     * @param aCbInterface Pointer to the callback interface
     */
    ServerPlugin( const QString& aPluginName,
                  const Profile& aProfile,
                  PluginCbInterface* aCbInterface );

    /*! \brief Destructor
     *
     */
    virtual ~ServerPlugin();

    /*! \brief Start listening for sync requests.
     *
     * Init must me called before this function.
     * @return True on success, otherwise false
     */
    virtual bool startListen() = 0;

    /*! \brief Stop listening for sync requests
     *
     */
    virtual void stopListen() = 0;

    /*! \brief Suspend activity
     *
     * Implement this if upon being asked to suspend for some reason, any ongoing
     * activity can be suspended
     */
    virtual void suspend() = 0;

    /*! \brief Resume suspended activity
     *
     */
    virtual void resume() = 0;

signals:

    /*! \brief Signal sent when a new sync session is received by the server
     *
     * @param aDestination Sync destination address, for example BT address
     *  or URL.
     */
    void newSession(const QString &aDestination);

protected:

    //! Profile Object that the server plugin operates on
    Profile iProfile;
};

}

#endif //SERVERPLUGIN_H_4399340943904399349843
