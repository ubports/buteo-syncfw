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

#include "PluginManager.h"

#include <QDir>
#include <QProcess>
#include <QPluginLoader>

#include <errno.h>
#include <signal.h>

#include "StoragePlugin.h"
#include "ServerPlugin.h"
#include "ClientPlugin.h"
#include "StorageChangeNotifierPlugin.h"
#include "OOPClientPlugin.h"
#include "OOPServerPlugin.h"
#include "SyncPluginLoader.h"
#include "StoragePluginLoader.h"
#include "StorageChangeNotifierPluginLoader.h"

#include "LogMacros.h"

namespace {
// Location filters of plugin maps
const QString STORAGEMAP_LOCATION = "-storage.so";
const QString CLIENTMAP_LOCATION = "-client.so";
const QString SERVERMAP_LOCATION = "-server.so";
const QString STORAGECHANGENOTIFIERMAP_LOCATION = "-changenotifier.so";

bool killProcess(const QString &exePath, const QStringList &args)
{
    const QByteArray expectedCmdLine = (exePath + args.join(QString())).toUtf8();
    const QString exeFullPath = QFileInfo(exePath).absolutePath();
    const QDir proc("/proc");

    QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        int pid = entry.toInt();
        if (pid) {
            QString exe = QFile::symLinkTarget(proc.filePath(entry).append("/exe"));
            if (exeFullPath == exe) {
                QFile cmdlineFile(proc.filePath(entry).append("/cmdline"));
                if (!cmdlineFile.open(QFile::ReadOnly)
                        || cmdlineFile.readAll() != expectedCmdLine) {
                    continue;
                }

                if (kill(pid, SIGTERM) == 0) {
                    LOG_DEBUG( "Process" << pid << "has been killed");
                    return true;
                } else {
                    LOG_WARNING("Failed to kill" << exePath << args <<
                                "[" << pid << "]" << strerror(errno));
                    return false;
                }
            }
        }
    }
    return false;
}


}

using namespace Buteo;

PluginManager::PluginManager()
    : PluginManager(QStringLiteral(DEFAULT_PLUGIN_PATH))
{
}

PluginManager::PluginManager(const QString &aPluginPath)
    : iPluginPath(aPluginPath)
{
    FUNCTION_CALL_TRACE;

    if (!iPluginPath.isEmpty() && !iPluginPath.endsWith('/')) {
        iPluginPath.append('/');
    }

    loadPluginMaps(iPluginPath, STORAGECHANGENOTIFIERMAP_LOCATION, iStorageChangeNotifierMaps);
    loadPluginMaps(iPluginPath, STORAGEMAP_LOCATION, iStorageMaps);
    loadPluginMaps(iPluginPath, CLIENTMAP_LOCATION, iClientMaps);
    loadPluginMaps(iPluginPath, SERVERMAP_LOCATION, iServerMaps);

    const QString ooppPath = iPluginPath + "oopp/";
    loadPluginMaps(ooppPath, CLIENTMAP_LOCATION, iOopClientMaps);
    loadPluginMaps(ooppPath, SERVERMAP_LOCATION, iOoPServerMaps);
}

PluginManager::~PluginManager()
{
    FUNCTION_CALL_TRACE;

    for (int i = 0; i < iLoadedDlls.count(); ++i) {
        iLoadedDlls[i].cleanUp();
    }
}

StorageChangeNotifierPlugin *PluginManager::createStorageChangeNotifier(const QString &aStorageName)
{
    FUNCTION_CALL_TRACE;

    if (!iStorageChangeNotifierMaps.contains(aStorageName)) {
        LOG_CRITICAL( "Library for the storage change notifier" << aStorageName << "does not exist" );
        return NULL;
    }

    QString libraryName = iStorageChangeNotifierMaps.value(aStorageName);

    if (StorageChangeNotifierPlugin * plugin
            = qobject_cast<StorageChangeNotifierPlugin *>(acquireLoadedPlugin(libraryName))) {
        return plugin;
    }

    QPluginLoader *pluginLoader = new QPluginLoader(libraryName, this);
    if (StorageChangeNotifierPluginLoader * notifierPluginLoader
            = qobject_cast<StorageChangeNotifierPluginLoader *>(pluginLoader->instance())) {
        StorageChangeNotifierPlugin *plugin = notifierPluginLoader->createPlugin(aStorageName);
        if (plugin) {
            addLoadedPlugin(libraryName, pluginLoader, plugin);
            return plugin;
        }
    }

    LOG_WARNING("Unable to load plugin " << libraryName << " from name " << aStorageName);
    pluginLoader->unload();
    delete pluginLoader;
    return NULL;
}

void PluginManager::destroyStorageChangeNotifier(StorageChangeNotifierPlugin *aPlugin)
{
    FUNCTION_CALL_TRACE;

    if (aPlugin == 0)
        return;

    QString storageName = aPlugin->name();

    if (!iStorageChangeNotifierMaps.contains(storageName)) {
        LOG_CRITICAL( "Library for the storage change notifier" << storageName << "does not exist" );
        return;
    }

    unloadPlugin(iStorageChangeNotifierMaps.value(storageName));
}

StoragePlugin *PluginManager::createStorage(const QString &aPluginName)
{
    FUNCTION_CALL_TRACE;

    if (!iStorageMaps.contains(aPluginName)) {
        LOG_CRITICAL( "Library for the storage" << aPluginName << "does not exist" );
        return NULL;
    }

    QString libraryName = iStorageMaps.value(aPluginName);

    if (StoragePlugin * plugin
            = qobject_cast<StoragePlugin *>(acquireLoadedPlugin(libraryName))) {
        return plugin;
    }

    QPluginLoader *pluginLoader = new QPluginLoader(libraryName, this);
    if (StoragePluginLoader * storagePluginLoader
            = qobject_cast<StoragePluginLoader *>(pluginLoader->instance())) {
        StoragePlugin *plugin = storagePluginLoader->createPlugin(aPluginName);
        if (plugin) {
            addLoadedPlugin(libraryName, pluginLoader, plugin);
            return plugin;
        }
    }

    LOG_WARNING("Unable to load plugin " << libraryName << " from name " << aPluginName);
    pluginLoader->unload();
    delete pluginLoader;
    return NULL;
}

void PluginManager::destroyStorage(StoragePlugin *aPlugin)
{
    FUNCTION_CALL_TRACE;

    if (aPlugin == 0)
        return;

    QString pluginName = aPlugin->getPluginName();

    if (!iStorageMaps.contains(pluginName)) {
        LOG_CRITICAL( "Library for the storage" << pluginName << "does not exist" );
        return;
    }

    unloadPlugin(iStorageMaps.value(pluginName));
}

ClientPlugin *PluginManager::createClient(const QString &aPluginName,
                                          const SyncProfile &aProfile,
                                          PluginCbInterface *aCbInterface)
{
    FUNCTION_CALL_TRACE;

    if (!iClientMaps.contains(aPluginName) &&
            !iOopClientMaps.contains(aPluginName)) {
        LOG_CRITICAL( "Library for the client" << aPluginName << "does not exist" );
        return NULL;
    }

    if (iClientMaps.contains(aPluginName)) {
        QString libraryName = iClientMaps.value(aPluginName);

        if (ClientPlugin * plugin
                = qobject_cast<ClientPlugin *>(acquireLoadedPlugin(libraryName))) {
            return plugin;
        }

        QPluginLoader *pluginLoader = new QPluginLoader(libraryName, this);
        if (SyncPluginLoader * syncPluginLoader
                = qobject_cast<SyncPluginLoader *>(pluginLoader->instance())) {
            ClientPlugin *plugin = syncPluginLoader->createClientPlugin(aPluginName, aProfile, aCbInterface);
            if (plugin) {
                addLoadedPlugin(libraryName, pluginLoader, plugin);
                return plugin;
            }
        }

        LOG_WARNING("Unable to load plugin " << libraryName << " from name " << aPluginName);
        pluginLoader->unload();
        delete pluginLoader;
        return NULL;

    } else if (iOopClientMaps.contains(aPluginName)) {
        // Start the out of process plugin
        const QString libraryName = iOopClientMaps.value(aPluginName);
        QProcess *process = startOOPPlugin(aPluginName, aProfile.name(), libraryName);

        if (process == NULL) {
            LOG_CRITICAL( "Could not start process" );
            return NULL;
        }

        // Create the client plugin interface to talk to the process
        return new OOPClientPlugin(aPluginName, aProfile, aCbInterface, *process);
    }

    return NULL;
}

void PluginManager::destroyClient(ClientPlugin *aPlugin)
{
    FUNCTION_CALL_TRACE;

    if (aPlugin == 0)
        return;

    QString pluginName = aPlugin->getPluginName();

    if (!iClientMaps.contains(pluginName) &&
            !iOopClientMaps.contains(pluginName)) {
        LOG_CRITICAL( "Library for the client plugin" << pluginName << "does not exist" );
        return;
    }

    if (iClientMaps.contains(pluginName)) {
        unloadPlugin(iClientMaps.value(pluginName));

    } else if (iOopClientMaps.contains(pluginName)) {
        // Stop the OOP process
        LOG_DEBUG( "Stopping the OOP process for " << pluginName);
        QString path = iOopClientMaps.value(pluginName);
        stopOOPPlugin(path);
        delete aPlugin;
    }
}

ServerPlugin *PluginManager::createServer(const QString &aPluginName,
                                          const Profile &aProfile,
                                          PluginCbInterface *aCbInterface)
{
    FUNCTION_CALL_TRACE;

    if (!iServerMaps.contains(aPluginName) &&
            !iOoPServerMaps.contains(aPluginName)) {
        LOG_CRITICAL( "Library for the server" << aPluginName << "does not exist" );
        return NULL;
    }

    if (iServerMaps.contains(aPluginName)) {
        // Load the plugin library
        QString libraryName = iServerMaps.value(aPluginName);

        if (ServerPlugin * plugin
                = qobject_cast<ServerPlugin *>(acquireLoadedPlugin(libraryName))) {
            return plugin;
        }

        QPluginLoader *pluginLoader = new QPluginLoader(libraryName, this);
        if (SyncPluginLoader * syncPluginLoader
                = qobject_cast<SyncPluginLoader *>(pluginLoader->instance())) {
            ServerPlugin *plugin = syncPluginLoader->createServerPlugin(aPluginName, aProfile, aCbInterface);
            if (plugin) {
                addLoadedPlugin(libraryName, pluginLoader, plugin);
                return plugin;
            }
        }

        LOG_WARNING("Unable to load plugin " << libraryName << " from name " << aPluginName);
        pluginLoader->unload();
        delete pluginLoader;
        return NULL;

    } else if (iOoPServerMaps.contains(aPluginName)) {
        // Start the Oop process plugin
        const QString libraryName = iOoPServerMaps.value(aPluginName);
        QProcess *process = startOOPPlugin(aPluginName, aProfile.name(), libraryName);

        if (process == NULL) {
            LOG_CRITICAL( "Could not start server plugin process" );
            return NULL;
        }

        return new OOPServerPlugin(aPluginName,
                                   aProfile,
                                   aCbInterface,
                                   *process);
    }

    return NULL;
}

void PluginManager::destroyServer(ServerPlugin *aPlugin)
{
    FUNCTION_CALL_TRACE;

    if (aPlugin == 0)
        return;

    QString pluginName = aPlugin->getPluginName();

    if (!iServerMaps.contains(pluginName) &&
            !iOoPServerMaps.contains(pluginName)) {
        LOG_CRITICAL( "Library for the server plugin" << pluginName << "does not exist" );
        return;
    }

    if (iServerMaps.contains(pluginName)) {
        // Unload the server plugin library
        unloadPlugin(iServerMaps.value(pluginName));

    } else if (iOoPServerMaps.contains(pluginName)) {
        // Stop the OOP server process
        QString path = iOoPServerMaps.value(pluginName);
        stopOOPPlugin(path);
        delete aPlugin;
    }
}

void PluginManager::loadPluginMaps(const QString &pluginDirPath, const QString &aFilter,
                                   QMap<QString, QString> &aTargetMap)
{
    FUNCTION_CALL_TRACE;

    QDir pluginDirectory(pluginDirPath);

    QStringList entries = pluginDirectory.entryList(QDir::Files);

    QStringList::const_iterator listIterator = entries.constBegin();
    while (listIterator != entries.constEnd()) {
        QString file = (*listIterator);

        if (!file.endsWith(aFilter)) {
            ++listIterator;
            continue;
        }
        // Remove filter from end
        file.chop(aFilter.length());

        // Remove lib
        file.remove(0, 3);

        aTargetMap[file] = pluginDirPath + (*listIterator);
        ++listIterator;
    }
}

QProcess *PluginManager::startOOPPlugin(const QString &aPluginName,
                                        const QString &aProfileName,
                                        const QString &aPluginFilePath)
{
    FUNCTION_CALL_TRACE;

    static const QString exePath = "/usr/libexec/buteo-oopp-runner";

    bool started = false;
    QStringList args;
    args << aPluginName << aProfileName << aPluginFilePath;

    if (killProcess(exePath, args)) {
        LOG_INFO( "Killed runaway plugin" << aProfileName);
    }

    LOG_DEBUG( "Starting oop plugin " << aProfileName);

    LOG_DEBUG( "Starting out-of-process plugin " << aPluginFilePath <<
               " with plugin name " << aPluginName <<
               " and profile name " << aProfileName);

    QProcess *process = new QProcess();
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->start(exePath, args);

    const QString clientPluginDBusServiceName(QString(QLatin1String("com.buteo.msyncd.plugin.profile-%1")).arg(
                                                  aProfileName));
    const QString serverPluginDBusServiceName(QString(QLatin1String("com.buteo.msyncd.plugin.%1")).arg(aProfileName));
    bool pluginHasRegistered = false;
    for (int i = 0; i < 30; i++) { // wait for up to thirty seconds for the process to register with dbus
        QThread::sleep(1);         // sleep for a second to wait for the process to be scheduled, init and register with dbus
        QDBusReply<bool> clientServiceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered(
                                                       clientPluginDBusServiceName);
        if (clientServiceRegistered.isValid() && clientServiceRegistered.value()) {
            pluginHasRegistered = true;
            break;
        }
        QDBusReply<bool> serverServiceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered(
                                                       serverPluginDBusServiceName);
        if (serverServiceRegistered.isValid() && serverServiceRegistered.value()) {
            pluginHasRegistered = true;
            break;
        }
    }

    if (process->state() == QProcess::Starting) {
        started = process->waitForStarted();
    } else {
        started = process->state() == QProcess::Running;
    }

    if (started) {
        DllInfo info;
        info.iPath = aPluginFilePath;
        info.iHandle = process;

        iDllLock.lockForWrite();
        iLoadedDlls.append(info);
        iDllLock.unlock();

        LOG_DEBUG( "Process " << process->program() << " started with pid " << process->pid() );
        if (!pluginHasRegistered) {
            LOG_DEBUG( "Process " << process->program() << " with pid " << process->pid() <<
                       "was unable to register DBus service: " << clientPluginDBusServiceName << "|" << serverPluginDBusServiceName );
        }
        connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
        return process;

    } else {
        LOG_CRITICAL("Unable to start process plugin " << aPluginFilePath <<
                     ". Error " << process->error());
        delete process;
        return NULL;
    }
}

void PluginManager::stopOOPPlugin(const QString &aPath)
{
    FUNCTION_CALL_TRACE;

    QProcess *process = NULL;

    iDllLock.lockForWrite();

    for (int i = 0; i < iLoadedDlls.size(); ++i) {
        if (iLoadedDlls[i].iPath == aPath) {
            process = iLoadedDlls[i].iHandle;
            break;
        }
    }

    iDllLock.unlock();

    // We must terminate the process outside of the locked section because
    // onProcessFinished handler below will want to acquire the same lock.
    // It will also schedule the deletion of the QProcess object.
    if (process) {
        process->terminate();
        if (process->waitForFinished() == false)
            process->kill();
    }
}

void PluginManager::onProcessFinished(int exitCode, QProcess::ExitStatus)
{
    FUNCTION_CALL_TRACE;

    QProcess *process = (QProcess *)sender();
    LOG_DEBUG( "Process " << process->program() << " finished with exit code" << exitCode );

    iDllLock.lockForWrite();

    for (int i = 0; i < iLoadedDlls.size(); ++i) {
        if (iLoadedDlls[i].iHandle == process) {
            iLoadedDlls.removeAt(i);
            break;
        }
    }

    iDllLock.unlock();

    process->deleteLater();
}

void PluginManager::addLoadedPlugin(const QString &libraryName,
                                    QPluginLoader *pluginLoader,
                                    QObject *plugin)
{
    DllInfo info;
    info.iPath = libraryName;
    info.iPluginLoader = pluginLoader;
    info.iLoadedPlugin = plugin;
    info.iRefCount = 1;

    iDllLock.lockForWrite();
    iLoadedDlls.append(info);
    iDllLock.unlock();
}

QObject *PluginManager::acquireLoadedPlugin(const QString &libraryName)
{
    QObject *plugin = nullptr;

    iDllLock.lockForWrite();
    for (int i = 0; i < iLoadedDlls.count(); ++i) {
        if (iLoadedDlls[i].iPath == libraryName) {
            iLoadedDlls[i].iRefCount++;
            plugin = iLoadedDlls[i].iLoadedPlugin;
            break;
        }
    }
    iDllLock.unlock();

    return plugin;
}

void PluginManager::unloadPlugin(const QString &libraryName)
{
    iDllLock.lockForWrite();

    for (int i = 0; i < iLoadedDlls.count(); ++i) {
        if (iLoadedDlls[i].iPath == libraryName && --iLoadedDlls[i].iRefCount == 0) {
            DllInfo info = iLoadedDlls.takeAt(i);
            info.cleanUp();
            break;
        }
    }

    iDllLock.unlock();
}

void PluginManager::DllInfo::cleanUp()
{
    delete iLoadedPlugin;

    if (iPluginLoader) {
        iPluginLoader->unload();
        delete iPluginLoader;
    }
}
