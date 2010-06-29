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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QString>
#include <QMap>
#include <QReadWriteLock>

namespace Buteo {

class StoragePlugin;
class ClientPlugin;
class ServerPlugin;
class PluginCbInterface;
class SyncProfile;
class Profile;

class ClientPluginTest;
class ServerPluginTest;
class StoragePluginTest;
    
/* !\brief Manages plugins
 *
 * Is responsible for creating and destroying storage,
 * server and client plugins.
 */
class PluginManager
{
public:
    static const QString DEFAULT_PLUGIN_PATH;


    /*! \brief Constructor
     *
     * @param aPluginPath Path where plugins are stored
     */
    PluginManager( const QString &aPluginPath = DEFAULT_PLUGIN_PATH );

    /*! \brief Destructor
     *
     */
    ~PluginManager();

    /*! \brief Creates a new storage plugin instance
     *
     * @param aName Name of the plugin
     * @return Storage plugin if success, otherwise NULL
     */
    StoragePlugin* createStorage( const QString& aPluginName );

    /*! \brief Destroys a storage plugin instance
     *
     * @param aPlugin Plugin to destroy
     */
    void destroyStorage( StoragePlugin* aPlugin );

    /*! \brief Creates a new client plugin instance
     *
     * @param aPluginName Name of the plugin
     * @param aProfile Sync profile
     * @param aCbInterface Callback interface
     * @return Client plugin on success, otherwise NULL
     */
    ClientPlugin* createClient( const QString& aPluginName,
                                const SyncProfile& aProfile,
                                PluginCbInterface *aCbInterface );

    /*! \brief Destroys a client plugin instance
     *
     * @param aPlugin Plugin to destroy
     */
    void destroyClient( ClientPlugin* aPlugin );

    /*! \brief Creates a new server plugin instance
     *
     * @param aPluginName Name of the plugin
     * @param aProfile Server profile
     * @param aCbInterface Callback interface
     * @return Server plugin on success, otherwise NULL
     */
    ServerPlugin* createServer( const QString& aPluginName,
                                const Profile& aProfile,
                                PluginCbInterface *aCbInterface );

    /*! \brief Destroys a server plugin
     *
     * @param aPlugin Plugin to destroy
     */
    void destroyServer( ServerPlugin *aPlugin );

private:

    struct DllInfo
    {
        QString iPath;
        void*   iHandle;
        int     iRefCount;

        DllInfo() : iHandle( NULL ), iRefCount( 0 ) { }
    };


    void loadPluginMaps( const QString aFilter, QMap<QString, QString>& aTargetMap );

    void* loadDll( const QString& aPath );

    void* getDllHandle( const QString& aPath );

    void unloadDll( const QString& aPath );

    QString                 iPluginPath;

    QMap<QString, QString>  iStorageMaps;
    QMap<QString, QString>  iClientMaps;
    QMap<QString, QString>  iServerMaps;

    QList<DllInfo>          iLoadedDlls;

    QReadWriteLock          iDllLock;

    friend class ClientPluginTest;
    friend class ServerPluginTest;
    friend class StoragePluginTest;

};

}

#endif
