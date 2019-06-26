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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QtDebug>
#include <QDateTime>

#include <dbus/dbus.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "LogMacros.h"
#include "Logger.h"
#include "synchronizer.h"
#include "SyncSigHandler.h"
#include "SyncCommonDefs.h"

Q_DECL_EXPORT int main( int argc, char* argv[] )
{
    // remove this later on if not needed in harmattan,
    // this IS needed for fremantle
    dbus_threads_init_default(); // magical line making program not crash

    QCoreApplication app(argc, argv);

    // The below two lines are added as a workaround for QT bug 11413
    // http://bugreports.qt.nokia.com/browse/QTBUG-11413
    QDBusConnection::sessionBus();
    QDBusConnection::systemBus();

    // Initialize the logger
    Buteo::Logger::instance();

    LOG_DEBUG("Starting Log At :"  << QDateTime::currentDateTime()  );

    Buteo::Synchronizer *synchronizer = new Buteo::Synchronizer(&app);
    if (synchronizer == 0) {
        LOG_FATAL("Failed to create synchronizer");
    }

    if(!synchronizer->initialize() ) {
        delete synchronizer;
        synchronizer = 0;
        return -1;
    }

    QString genericCache = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    QFile::Permissions permissions(QFile::ExeOwner | QFile::ExeGroup | QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup);

    // Make sure we have the .cache directory
    QDir homeDir(genericCache);
    if (homeDir.mkpath(genericCache)) {
        uid_t uid = getuid();
        struct passwd *pwd;
        struct group *grp;
        // assumes that correct groupname is same as username (e.g. nemo:nemo)
        pwd = getpwuid(uid);
        if (pwd != NULL) {
            grp = getgrnam(pwd->pw_name);
            if (grp != NULL) {
                gid_t gid = grp->gr_gid;
                int ret = chown(genericCache.toLatin1().data(), uid, gid);
                Q_UNUSED(ret);
            }
        }
        QFile::setPermissions(genericCache, permissions);
    }

    QString msyncCacheSyncDir = Sync::syncCacheDir() + QDir::separator() + "sync";

    // Make sure we have the msyncd/sync directory
    QDir syncDir(msyncCacheSyncDir);
    if (syncDir.mkpath(msyncCacheSyncDir)) {
        QFile::setPermissions(Sync::syncCacheDir(), permissions);
        QFile::setPermissions(msyncCacheSyncDir, permissions);
    }

    //Note:- Since we can't call Qt functions from Unix signal handlers.
    // This class provide hanlding unix  signal.
    SyncSigHandler *sigHandler = new SyncSigHandler();

    LOG_DEBUG("Entering event loop");
    int returnValue = app.exec();
    LOG_DEBUG("Exiting event loop");

    synchronizer->close();
    delete synchronizer;
    synchronizer = 0;

    if (sigHandler) {
        delete sigHandler;
        sigHandler = 0;
    }

    LOG_DEBUG("Stopping logger");

    Buteo::Logger::deleteInstance();

    qDebug() << "Exiting program";

    return returnValue;
}
