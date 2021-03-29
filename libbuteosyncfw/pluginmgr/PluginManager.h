/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2014-2021 Jolla Ltd.
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
#include <QProcess>
#include <QPointer>

class QPluginLoader;
class QProcess;

namespace Buteo {

class StorageChangeNotifierPlugin;
class StoragePlugin;
class ClientPlugin;
class ServerPlugin;
class PluginCbInterface;
class SyncProfile;
class Profile;

class ClientPluginTest;
class ServerPluginTest;
class StoragePluginTest;

/*! \brief Manages plugins
 *
 * Is responsible for creating and destroying storage,
 * server and client plugins.
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    PluginManager();
    /*! \brief Constructor
     *
     * @param aPluginPath Path where plugins are stored
     */
    PluginManager( const QString &aPluginPath );

    /*! \brief Destructor
     *
     */
    ~PluginManager();

    /*! \brief Creates a new storage change notifier plugin
     * for the storage aStoragName
     *
     * @param aStorageName well-known name of the storage
     */
    StorageChangeNotifierPlugin* createStorageChangeNotifier( const QString& aStorageName );

    /*! \brief Destroys a storage change notifier plugin instance
     *
     * @param aStorageName well-known storage name of the plugin to be destroyed
     */
    void destroyStorageChangeNotifier( StorageChangeNotifierPlugin *aPlugin );

    /*! \brief Creates a new storage plugin instance
     *
     * @param aPluginName Name of the plugin
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

protected slots:

    void onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

private:

    class DllInfo
    {
    public:
        void cleanUp();

        QString iPath;
        QProcess *iHandle = nullptr;
        QPluginLoader *iPluginLoader = nullptr;
        QPointer<QObject> iLoadedPlugin;
        int iRefCount = 0;
    };

    void loadPluginMaps(const QString &pluginDirPath, const QString &aFilter, QMap<QString, QString>& aTargetMap );

    QProcess* startOOPPlugin( const QString& aPluginName, const QString& aProfileName, const QString& aPluginFilePath);

    void stopOOPPlugin( const QString& aPath );

    void addLoadedPlugin(const QString &libraryName,
                         QPluginLoader *pluginLoader,
                         QObject *plugin);
    QObject *acquireLoadedPlugin(const QString &libraryName);
    void unloadPlugin(const QString &libraryName);

    QString                 iPluginPath;

    QMap<QString, QString>  iStorageChangeNotifierMaps;
    QMap<QString, QString>  iStorageMaps;
    QMap<QString, QString>  iClientMaps;
    QMap<QString, QString>  iServerMaps;

    QMap<QString, QString>  iOopClientMaps;
    QMap<QString, QString>  iOoPServerMaps;

    QList<DllInfo>          iLoadedDlls;

    QReadWriteLock          iDllLock;

    QString                 iProcBinaryPath;

#ifdef SYNCFW_UNIT_TESTS
    friend class ClientPluginTest;
    friend class ServerPluginTest;
    friend class StoragePluginTest;
#endif

};

}

#endif
