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

#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <QCoreApplication>


#include "SyncSigHandler.h"
#include "LogMacros.h"

int SyncSigHandler::iSigHupFd[2];
int SyncSigHandler::iSigTermFd[2];

SyncSigHandler::SyncSigHandler(QObject *aParent, const char */*aName*/)
    : QObject(aParent)
{
    FUNCTION_CALL_TRACE;

    //Adding signal hanlder for unix Signals
    signal(SIGTERM, termSignalHandler);
    signal(SIGINT, termSignalHandler);
    signal(SIGHUP, hupSignalHandler);

    //Adding socketpair to monitor those fd's.
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, iSigHupFd)) {
        LOG_CRITICAL("Couldn't create HUP socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, iSigTermFd)) {
        LOG_CRITICAL("Couldn't create TERM socketpair");
    }

    //SocketNotifier for read those fd's.
    iSigHup = new QSocketNotifier(iSigHupFd[1], QSocketNotifier::Read, this);
    connect(iSigHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));

    iSigTerm = new QSocketNotifier(iSigTermFd[1], QSocketNotifier::Read, this);
    connect(iSigTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
}

SyncSigHandler::~SyncSigHandler()
{
    FUNCTION_CALL_TRACE;
    if (iSigHup) {
        delete iSigHup;
        iSigHup = 0;
    }
    if (iSigTerm) {
        delete iSigTerm;
        iSigTerm = 0;
    }
}

// Linux signal handler.
//This application is shutdown by sending a SIGTERM to it.
void SyncSigHandler::termSignalHandler(int /*signal*/)
{
    char a = 1;
    ::write(iSigTermFd[0], &a, sizeof(a));
}

void SyncSigHandler::hupSignalHandler(int /*signal*/)
{
    // Do nothing
}

//Qt Slot will eventually get called corresponding to Unix signal.
void SyncSigHandler::handleSigTerm()
{
    FUNCTION_CALL_TRACE;
    iSigTerm->setEnabled(false);
    char tmp;
    ::read(iSigTermFd[1], &tmp, sizeof(tmp));

    // Doing Qt stuff.Exiting application
    QCoreApplication::exit(0);
}

void SyncSigHandler::handleSigHup()
{
    FUNCTION_CALL_TRACE;
}
