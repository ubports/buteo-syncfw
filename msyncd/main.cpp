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
#include <QtDebug>
#include <QDateTime>

#include <dbus/dbus.h>

#include "LogMacros.h"
#include "Logger.h"
#include "synchronizer.h"
#include "SyncSigHandler.h"

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

    // This could be enabled to log the output to file
    // Disabling the output to file for now. To get the output, run msyncd from
    // cmd-line
    //setLogLevelFromFile();

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
