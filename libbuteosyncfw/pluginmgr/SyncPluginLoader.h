/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2021 Jolla Ltd.
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
#ifndef SYNCPLUGINLOADER_H
#define SYNCPLUGINLOADER_H

#include "SyncProfile.h"

#include <QObject>

namespace Buteo {

class PluginCbInterface;
class ClientPlugin;
class ServerPlugin;
class SyncPluginBase;

/*! \brief Base class for sync plugin loaders
 *
 */
class SyncPluginLoader : public QObject
{
    Q_OBJECT
public:
    virtual ClientPlugin *createClientPlugin(const QString &aPluginName,
                                             const SyncProfile &aProfile,
                                             PluginCbInterface *aCbInterface);

    virtual ServerPlugin *createServerPlugin(const QString &aPluginName,
                                             const Profile &aProfile,
                                             PluginCbInterface *aCbInterface);
};

}

Q_DECLARE_INTERFACE(Buteo::SyncPluginLoader, "com.buteo.msyncd.SyncPluginLoader/1.0")

#endif  //  SYNCPLUGINLOADER_H
