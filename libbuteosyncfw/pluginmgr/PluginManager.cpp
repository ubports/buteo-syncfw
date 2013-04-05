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

#include "PluginManager.h"

#include <QDir>

#include <dlfcn.h>

#include "StoragePlugin.h"
#include "ServerPlugin.h"
#include "ClientPlugin.h"
#include "StorageChangeNotifierPlugin.h"

#include "LogMacros.h"

using namespace Buteo;

PluginManager::PluginManager( const QString &aPluginPath )
 : iPluginPath( aPluginPath )
{
    FUNCTION_CALL_TRACE;
    
    if (!iPluginPath.isEmpty() && !iPluginPath.endsWith('/'))
    {
        iPluginPath.append('/');
    }

    loadPluginMaps( STORAGECHANGENOTIFIERMAP_LOCATION, iStorageChangeNotifierMaps );
    loadPluginMaps( STORAGEMAP_LOCATION, iStorageMaps );
    loadPluginMaps( CLIENTMAP_LOCATION, iClientMaps );
    loadPluginMaps( SERVERMAP_LOCATION, iServerMaps );
}

PluginManager::~PluginManager()
{
    FUNCTION_CALL_TRACE;

    if( !iLoadedDlls.isEmpty() ) {
        LOG_WARNING( "Plugin manager: found" << iLoadedDlls.count() << "libraries not properly destroyed:" );

        for( int i = 0; i < iLoadedDlls.count(); ++i ) {
            LOG_WARNING( iLoadedDlls[i].iPath );
        }
    }

}

StorageChangeNotifierPlugin* PluginManager::createStorageChangeNotifier( const QString& aStorageName )
{
    FUNCTION_CALL_TRACE;

    if( ! iStorageChangeNotifierMaps.contains(aStorageName) )
    {
        LOG_CRITICAL( "Library for the storage change notifier" << aStorageName << "does not exist" );
        return NULL;
    }

    QString libraryName = iStorageChangeNotifierMaps.value(aStorageName);

    void* handle = loadDll( libraryName );

    if( !handle ) {
        return NULL;
    }

    FUNC_CREATE_STORAGECHANGENOTIFIER storageChangeNotifierPointer = ( FUNC_CREATE_STORAGECHANGENOTIFIER)dlsym(
                                                                       handle,
                                                                       CREATE_FUNCTION.toStdString().
                                                                       c_str() );

    if( dlerror() )
    {
        LOG_CRITICAL( "Library" << libraryName << "does not have a create function" );
        unloadDll( libraryName );
        return NULL;
    }

    StorageChangeNotifierPlugin* plugin = (*storageChangeNotifierPointer)(aStorageName);

    if( plugin ) {
        return plugin;
    }
    else {
        LOG_CRITICAL( "Could not create plugin instance" );
        unloadDll( libraryName );
        return NULL;
    }
}

void PluginManager::destroyStorageChangeNotifier( StorageChangeNotifierPlugin* aPlugin )
{
    FUNCTION_CALL_TRACE;

    if ( aPlugin == 0 )
        return;

    QString storageName = aPlugin->name();

    if ( ! iStorageChangeNotifierMaps.contains(storageName) ) {
        LOG_CRITICAL( "Library for the storage change notifier" << storageName << "does not exist" );
        return;
    }

    QString path = iStorageChangeNotifierMaps.value(storageName);

    void* handle = getDllHandle( path );

    if( !handle ) {
        LOG_CRITICAL( "Could not find library for storage plugin" << storageName );
        return;
    }

    FUNC_DESTROY_STORAGECHANGENOTIFIER storageChangeNotifierDestroyer = (FUNC_DESTROY_STORAGECHANGENOTIFIER)dlsym(
                                                                         handle,
                                                                         DESTROY_FUNCTION.toStdString().
                                                                         c_str());

    if (dlerror()) {
        unloadDll( path );
        LOG_CRITICAL( "Library" << path << "does not have a destroy function" );
    }
    else {
        (*storageChangeNotifierDestroyer)(aPlugin);
        unloadDll( path );
    }

}
StoragePlugin* PluginManager::createStorage( const QString& aPluginName )
{
    FUNCTION_CALL_TRACE;

    if( ! iStorageMaps.contains(aPluginName) )
    {
        LOG_CRITICAL( "Library for the storage" << aPluginName << "does not exist" );
        return NULL;
    }

    QString libraryName = iStorageMaps.value(aPluginName);

    void* handle = loadDll( libraryName );

    if( !handle ) {
        return NULL;
    }

    FUNC_CREATE_STORAGE storagePointer = ( FUNC_CREATE_STORAGE)dlsym(
                                           handle,
                                           CREATE_FUNCTION.toStdString().
                                           c_str() );

    if( dlerror() )
    {
        LOG_CRITICAL( "Library" << libraryName << "does not have a create function" );
        unloadDll( libraryName );
        return NULL;
    }

    StoragePlugin* plugin = (*storagePointer)(aPluginName);

    if( plugin ) {
        return plugin;
    }
    else {
        LOG_CRITICAL( "Could not create plugin instance" );
        unloadDll( libraryName );
        return NULL;
    }

}

void PluginManager::destroyStorage( StoragePlugin* aPlugin )
{
    FUNCTION_CALL_TRACE;

    if ( aPlugin == 0 )
        return;

    QString pluginName = aPlugin->getPluginName();

    if ( ! iStorageMaps.contains(pluginName)) {
        LOG_CRITICAL( "Library for the storage" << pluginName << "does not exist" );
        return;
    }

    QString path = iStorageMaps.value(pluginName);

    void* handle = getDllHandle( path );

    if( !handle ) {
        LOG_CRITICAL( "Could not find library for storage plugin" << pluginName );
        return;
    }

    FUNC_DESTROY_STORAGE storageDestroyer = (FUNC_DESTROY_STORAGE)dlsym(
                                             handle,
                                             DESTROY_FUNCTION.toStdString().
                                             c_str());

    if (dlerror()) {
        unloadDll( path );
        LOG_CRITICAL( "Library" << path << "does not have a destroy function" );
    }
    else {
        (*storageDestroyer)(aPlugin);
        unloadDll( path );
    }

}

ClientPlugin* PluginManager::createClient( const QString& aPluginName,
                                           const SyncProfile& aProfile,
                                           PluginCbInterface *aCbInterface)
{

    FUNCTION_CALL_TRACE;

    if( ! iClientMaps.contains(aPluginName) )
    {
        LOG_CRITICAL( "Library for the client" << aPluginName << "does not exist" );
        return NULL;
    }

    QString libraryName = iClientMaps.value(aPluginName);

    void* handle = loadDll( libraryName );

    if( !handle ) {
        return NULL;
    }

    FUNC_CREATE_CLIENT clientPointer = (FUNC_CREATE_CLIENT)dlsym( handle,
                                        CREATE_FUNCTION.toStdString().c_str() );

    if( dlerror() )
    {
        LOG_CRITICAL( "Library" << libraryName << "does not have a create function" );
        unloadDll( libraryName );
        return NULL;
    }

    ClientPlugin* plugin = (*clientPointer)( aPluginName, aProfile, aCbInterface );

    if( plugin ) {
        return plugin;
    }
    else {
        LOG_CRITICAL( "Could not create plugin instance" );
        unloadDll( libraryName );
        return NULL;
    }

}

void PluginManager::destroyClient( ClientPlugin *aPlugin )
{
    FUNCTION_CALL_TRACE;

    if ( aPlugin == 0 )
        return;

    QString pluginName = aPlugin->getPluginName();

    if ( ! iClientMaps.contains(pluginName) ) {
        LOG_CRITICAL( "Library for the client plugin" << pluginName << "does not exist" );
        return;
    }

    QString path = iClientMaps.value(pluginName);

    void* handle = getDllHandle( path );

    if( !handle ) {
        LOG_CRITICAL( "Could not find library for client plugin" << pluginName );
        return;
    }

    FUNC_DESTROY_CLIENT clientDestroyer = (FUNC_DESTROY_CLIENT)dlsym(
                                           handle,
                                           DESTROY_FUNCTION.toStdString().c_str() );

    if (dlerror()) {
        unloadDll( path );
        LOG_CRITICAL( "Library" << path << "does not have a destroy function" );
    }
    else {
        (*clientDestroyer)(aPlugin);
        unloadDll( path );
    }

}

ServerPlugin* PluginManager::createServer( const QString& aPluginName,
                                           const Profile& aProfile,
                                           PluginCbInterface *aCbInterface )
{
    FUNCTION_CALL_TRACE;

    if( ! iServerMaps.contains(aPluginName) )
    {
        LOG_CRITICAL( "Library for the server" << aPluginName << "does not exist" );
        return NULL;
    }

    QString libraryName = iServerMaps.value(aPluginName);

    void* handle = loadDll( libraryName );

    if( !handle ) {
        LOG_CRITICAL("Loading library failed");
        return NULL;
    }

    FUNC_CREATE_SERVER serverPointer = (FUNC_CREATE_SERVER)dlsym( handle,
                                        CREATE_FUNCTION.toStdString().c_str());

    if( dlerror() )
    {
        LOG_CRITICAL( "Library" << libraryName << "does not have a create function" );
        unloadDll( libraryName );
        return NULL;
    }

    ServerPlugin* plugin = (*serverPointer)( aPluginName, aProfile, aCbInterface );

    if( plugin ) {
        return plugin;
    }
    else {
        LOG_CRITICAL( "Could not create plugin instance" );
        unloadDll( libraryName );
        return NULL;
    }

}

void PluginManager::destroyServer( ServerPlugin *aPlugin )
{
    FUNCTION_CALL_TRACE;
    
    if ( aPlugin == 0 )
        return;
    
    QString pluginName = aPlugin->getPluginName();

    if ( ! iServerMaps.contains(pluginName) ) {
        LOG_CRITICAL( "Library for the server plugin" << pluginName << "does not exist" );
        return;
    }

    QString path = iServerMaps.value(pluginName);

    void* handle = getDllHandle( path );

    if( !handle ) {
        LOG_CRITICAL( "Could not find library for server plugin" << pluginName );
        return;
    }

    FUNC_DESTROY_SERVER serverDestroyer = (FUNC_DESTROY_SERVER)dlsym(
                                           handle,
                                           DESTROY_FUNCTION.toStdString().c_str());

    if (dlerror()) {
        unloadDll( path );
        LOG_CRITICAL( "Library" << path << "does not have a destroy function" );
    }
    else {
        (*serverDestroyer)(aPlugin);
        unloadDll( path );
    }
}

void PluginManager::loadPluginMaps( const QString aFilter, QMap<QString, QString>& aTargetMap )
{
    FUNCTION_CALL_TRACE;

    QDir pluginDirectory( iPluginPath );

    QStringList entries = pluginDirectory.entryList( QDir::Files );

    QStringList::const_iterator listIterator = entries.constBegin();
    while (listIterator != entries.constEnd())
    {
        QString file = (*listIterator);

        if (!file.endsWith(aFilter))
        {
            ++listIterator;
            continue;
        }
        // Remove filter from end
        file.chop( aFilter.length() );

        // Remove lib
        file.remove(0, 3);
        aTargetMap[file] = iPluginPath + (*listIterator);
        ++listIterator;
    }

}

void* PluginManager::loadDll( const QString& aPath )
{
    
    FUNCTION_CALL_TRACE;
    iDllLock.lockForWrite();

    void* handle = NULL;

    LOG_DEBUG( "Searching for DLL:" << aPath );

    for( int i = 0; i < iLoadedDlls.count(); ++i ) {

        if( iLoadedDlls[i].iPath == aPath ) {
            LOG_DEBUG( "DLL already loaded:" << aPath );
            handle = iLoadedDlls[i].iHandle;
            ++iLoadedDlls[i].iRefCount;
        }
    }

    if( !handle ) {

        LOG_DEBUG( "Opening DLL:" << aPath );

        handle = dlopen( aPath.toStdString().c_str(), RTLD_NOW );

        if( handle ) {
            DllInfo info;
            info.iPath = aPath;
            info.iHandle = handle;
            info.iRefCount = 1;
            iLoadedDlls.append( info );
        }
        else {
            LOG_CRITICAL( "Cannot load library " << aPath <<":" << dlerror() );
        }

    }

    iDllLock.unlock();

    return handle;
}

void* PluginManager::getDllHandle( const QString& aPath )
{
    FUNCTION_CALL_TRACE;
    
    iDllLock.lockForRead();

    void* handle = NULL;

    for( int i = 0; i < iLoadedDlls.count(); ++i ) {

        if( iLoadedDlls[i].iPath == aPath ) {

            handle = iLoadedDlls[i].iHandle;
            break;

        }

    }

    iDllLock.unlock();

    return handle;
}

void PluginManager::unloadDll( const QString& aPath )
{
    FUNCTION_CALL_TRACE;

    iDllLock.lockForWrite();

    for( int i = 0; i < iLoadedDlls.count(); ++i ) {

        if( iLoadedDlls[i].iPath == aPath ) {
            --iLoadedDlls[i].iRefCount;
#if 0
KLUDGE: Due to NB #169065, crashes are seen in QMetaType if we unload DLLs. Hence commenting
            this code out for now.
            if( iLoadedDlls[i].iRefCount == 0 ) {
                dlclose( iLoadedDlls[i].iHandle );
                iLoadedDlls.removeAt( i );
            }
#endif
            break;

        }

    }

    iDllLock.unlock();

}
