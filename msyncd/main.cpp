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

#include <signal.h>
#include <dbus/dbus.h>

#include "LogMacros.h"
#include "Logger.h"
#include "synchronizer.h"

const QString LOGGER_CONFIG_FILE( "/etc/sync/set_sync_log_level" );
const QString SYNC_LOG_FILE_PATH( QDir::homePath () + QDir::separator() + ".sync" + QDir::separator() + "synchronizer.log");

// Linux signal handler. This application is shutdown by sending a
// SIGTERM to it
void signalHandler(int /*signal*/)
{
    QCoreApplication::exit(0);
}

void setLogLevelFromFile(Buteo::Logger *aLogger)
{
    if(QFile::exists(LOGGER_CONFIG_FILE)) {
    	// read the config level from the file and set
    	// that level
    	QFile file(LOGGER_CONFIG_FILE);
    	if(file.open(QIODevice::ReadOnly)) {
            int level = file.readLine().simplified().toInt();
    		qDebug()  << "Setting Log Level to " << level;
    		if(!aLogger->setLogLevel(level)) {
    			qDebug() << "Invalid Log Level Read from the file" ;
    		}
    		file.close();
    	}
    }
}

int main( int argc, char* argv[] )
{
    // remove this later on if not needed in harmattan,
    // this IS needed for fremantle
    dbus_threads_init_default(); // magical line making program not crash

    QCoreApplication app(argc, argv);

    Buteo::Logger::createInstance(SYNC_LOG_FILE_PATH);
    Buteo::Logger *logger = Buteo::Logger::instance();
    if (logger) {
        setLogLevelFromFile(logger);
        qDebug() << "Current log level is " <<
            (logger->getLogLevelArray().count(true) - 1);
        qDebug() << "Logs will be logged to " << SYNC_LOG_FILE_PATH;
    }

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

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    LOG_DEBUG("Entering event loop");
    int returnValue = app.exec();
    LOG_DEBUG("Exiting event loop");

    synchronizer->close();
    delete synchronizer;
    synchronizer = 0;

    LOG_DEBUG("Stopping logger");

    Buteo::Logger::deleteInstance();

    qDebug() << "Exiting program";

    return returnValue;
}
