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
#ifndef CLIENTPLUGIN_H
#define CLIENTPLUGIN_H

#include "SyncPluginBase.h"
#include "libsyncprofile/SyncProfile.h"
#include <QMetaType>

namespace Buteo {

class PluginCbInterface;

/*! \brief Base class for client plugins
 *
 */
class ClientPlugin : public SyncPluginBase
{
    Q_OBJECT;

public:

    /*! \brief Constructor
     *
     * @param aPluginName Name of this client plugin
     * @param aProfile Sync profile for the client
     * @param aCbInterface Pointer to the callback interface
     */
    ClientPlugin( const QString& aPluginName,
                  const SyncProfile& aProfile,
                  PluginCbInterface* aCbInterface );

    /*! \brief Destructor
     *
     * Call uninit before destroying the client plug-in.
     */
    virtual ~ClientPlugin();

    /*! \brief Starts synchronization
     *
     * Init must be called before calling this function.
     * @returns True on success, otherwise false
     */
    virtual bool startSync() = 0;

    /*! \brief access to profile owned and used by this instance
     */
    SyncProfile &profile() { return iProfile; }

protected:

    //! Sync Profile Object that the plugin is currently operating on
    SyncProfile iProfile;

private:

};

}

#endif  //  CLIENTPLUGIN_H
