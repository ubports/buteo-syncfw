/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2014 Jolla Ltd
 *
 * Contact: Valerio Valerio <valerio.valerio@jolla.com>
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

#include "LogMacros.h"
#include "BackgroundSync.h"
#include <QStringList>
#include <QDebug>
#include <keepalive/backgroundactivity.h>

using namespace Buteo;

BackgroundSync::BackgroundSync(QObject* aParent)
 :  QObject(aParent)
{
    FUNCTION_CALL_TRACE;
}

BackgroundSync::~BackgroundSync()
{
    FUNCTION_CALL_TRACE;

    removeAllBackgroundSync();
}

void BackgroundSync::removeAllBackgroundSync()
{
    FUNCTION_CALL_TRACE;

    QStringList profNames;

    QMapIterator<QString,BActivityStruct> iter(iScheduledSyncs);
    while (iter.hasNext()) {
        iter.next();
        profNames.append(iter.key());
    }

    for(int i=0; i<profNames.size(); i++) {
        removeBackgroundSync(profNames[i]);
    }
}

void BackgroundSync::removeBackgroundSync(const QString &aProfName)
{
    FUNCTION_CALL_TRACE;

    if(iScheduledSyncs.contains(aProfName) == false)
        return;

    BActivityStruct& tmp = iScheduledSyncs[aProfName];

    tmp.backgroundActivity->stop();
    delete tmp.backgroundActivity;

    iScheduledSyncs.remove(aProfName);
}

QString BackgroundSync::getProfNameFromId(const QString activityId)
{
    FUNCTION_CALL_TRACE;

    QMapIterator<QString,BActivityStruct> iter(iScheduledSyncs);

    while (iter.hasNext()) {
        iter.next();
        const BActivityStruct& tmp = iter.value();
        if(tmp.id == activityId) {
            return iter.key();
            break;
        }
    }

    return QString();
}

bool BackgroundSync::setBackgroundSync(const QString &aProfName)
{
    FUNCTION_CALL_TRACE;

    if( aProfName.isEmpty())
        return false;

    if(iScheduledSyncs.contains(aProfName) == true) {
        LOG_DEBUG("Profile already in waiting... No new BackgroundSync");
        return true; //returing 'true' - no immediate sync request to be sent.
    }

    BActivityStruct &newAct = iScheduledSyncs[aProfName];
    newAct.backgroundActivity = new BackgroundActivity(this);
    newAct.id = newAct.backgroundActivity->id();
    connect(newAct.backgroundActivity,SIGNAL(running()), this, SLOT(onBackgroundSyncStarted()));
    newAct.backgroundActivity->run();

    LOG_DEBUG("setBackgroundSync(), profile name = " << aProfName);
    return true;
}

void BackgroundSync::onBackgroundSyncStarted()
{
    FUNCTION_CALL_TRACE;

    BackgroundActivity *tempAct = static_cast<BackgroundActivity*>(sender());

    QString profName = getProfNameFromId(tempAct->id());

    if (!profName.isEmpty()) {
        LOG_DEBUG("Background Sync started, for profile = " << profName);
        emit onBackgroundSyncRunning(profName);
    } else {
        LOG_DEBUG("Error profile for background activity not found");
    }
}

void BackgroundSync::onBackgroundSyncCompleted(QString aProfName)
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Background sync completed, removing activity, profile name = " << aProfName);
    removeBackgroundSync(aProfName);
}


