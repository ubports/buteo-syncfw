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
#include "IPHeartBeat.h"
#include <QStringList>
#include "LogMacros.h"

using namespace Buteo;

IPHeartBeat::IPHeartBeat(QObject* aParent)
 :  QObject(aParent)
{
    FUNCTION_CALL_TRACE;
}

IPHeartBeat::~IPHeartBeat()
{
    FUNCTION_CALL_TRACE;

    removeAllWaits();
}

void IPHeartBeat::removeAllWaits()
{
    FUNCTION_CALL_TRACE;

    QStringList profNames;

    QMapIterator<QString,BeatStruct> iter(iBeatsWaiting);
    while (iter.hasNext()) {
        iter.next();
        profNames.append(iter.key());
    }

    for(int i=0; i<profNames.size(); i++) {
        removeWait(profNames[i]);
    }
}

void IPHeartBeat::removeWait(const QString& aProfName)
{
    FUNCTION_CALL_TRACE;

    if(iBeatsWaiting.contains(aProfName) == false)
        return;

    BeatStruct& tmp = iBeatsWaiting[aProfName];

    delete tmp.sockNotifier;

    iphb_discard_wakeups(tmp.iphbHandle);
    iphb_close(tmp.iphbHandle);

    iBeatsWaiting.remove(aProfName);
}

bool IPHeartBeat::getProfNameFromFd(int aSockFd, QString& aProfName)
{
    FUNCTION_CALL_TRACE;

    bool ret = false;

    QMapIterator<QString,BeatStruct> iter(iBeatsWaiting);

    while (iter.hasNext()) {
        iter.next();
        const BeatStruct& tmp = iter.value();
        if(tmp.sockfd == aSockFd) {
            aProfName = iter.key();
            ret = true;
            break;
        }
    }

    return ret;
}

bool IPHeartBeat::setHeartBeat(const QString& aProfName, ushort aMinWaitTime, ushort aMaxWaitTime)
{
    FUNCTION_CALL_TRACE;

    if( (aMinWaitTime > aMaxWaitTime) || aProfName.isEmpty() )
        return false;

    LOG_DEBUG("setHeartBeat(), profile name = " << aProfName);

    if(iBeatsWaiting.contains(aProfName) == true) {
        LOG_DEBUG("Profile already in waiting... No new beat");
        return true; //returing 'true' - no immediate sync request to be sent.
    }

    iphb_t iphbHandle = iphb_open(NULL);

    if(iphbHandle == 0) {
        LOG_DEBUG("iphb_open() failed.... No IP heartbeat available");
        return false;
    }

    int sockfd = iphb_get_fd(iphbHandle);

    if(sockfd < 0) {
        LOG_DEBUG("iphb_get_fd() failed.... No IP heartbeat");
        iphb_close(iphbHandle);
        return false;
    }

    BeatStruct& newBeat = iBeatsWaiting[aProfName];
    newBeat.iphbHandle = iphbHandle;
    newBeat.sockfd = sockfd;
    newBeat.sockNotifier = new QSocketNotifier(sockfd,QSocketNotifier::Read,0);

    if(iphb_wait(iphbHandle,aMinWaitTime,aMaxWaitTime,0) == -1) {
        LOG_DEBUG("iphb_wait() failed .... No IP heartbeat");
        removeWait(aProfName);
        return false;
    }

    connect(newBeat.sockNotifier,SIGNAL(activated(int)),this,SLOT(internalBeatTriggered(int)));

    LOG_DEBUG("IP Heartbeat set for profile");

    return true;
}

void IPHeartBeat::internalBeatTriggered(int aSockFd)
{
    FUNCTION_CALL_TRACE;

    QString profName;

    if(getProfNameFromFd(aSockFd,profName) == true) {
        removeWait(profName);
        LOG_DEBUG("Emitting IP  Heartbeat, profile name = " << profName);
        emit onHeartBeat(profName);
    }
}
