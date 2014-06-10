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

#include "SyncDBusAdaptor.h"
#include "synchronizer.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

using namespace Buteo;

/*
 * Implementation of adaptor class SyncDBusAdaptor
 */

SyncDBusAdaptor::SyncDBusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

SyncDBusAdaptor::~SyncDBusAdaptor()
{
    // destructor
}

void SyncDBusAdaptor::abortSync(const QString &aProfileId)
{
    // handle method call com.meego.msyncd.abortSync
    QMetaObject::invokeMethod(parent(), "abortSync", Q_ARG(QString, aProfileId));
}

QStringList SyncDBusAdaptor::allVisibleSyncProfiles()
{
    // handle method call com.meego.msyncd.allVisibleSyncProfiles
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "allVisibleSyncProfiles", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

bool SyncDBusAdaptor::getBackUpRestoreState()
{
    // handle method call com.meego.msyncd.getBackUpRestoreState
    bool out0;
    QMetaObject::invokeMethod(parent(), "getBackUpRestoreState", Q_RETURN_ARG(bool, out0));
    return out0;
}

QString SyncDBusAdaptor::getLastSyncResult(const QString &aProfileId)
{
    // handle method call com.meego.msyncd.getLastSyncResult
    QString out0;
    QMetaObject::invokeMethod(parent(), "getLastSyncResult", Q_RETURN_ARG(QString, out0), Q_ARG(QString, aProfileId));
    return out0;
}

bool SyncDBusAdaptor::isConnectivityAvailable(int connectivityType)
{
    // handle method call com.meego.msyncd.isConnectivityAvailable
    bool out0;
    QMetaObject::invokeMethod(parent(), "isConnectivityAvailable", Q_RETURN_ARG(bool, out0), Q_ARG(int, connectivityType));
    return out0;
}

void SyncDBusAdaptor::releaseStorages(const QStringList &aStorageNames)
{
    // handle method call com.meego.msyncd.releaseStorages
    QMetaObject::invokeMethod(parent(), "releaseStorages", Q_ARG(QStringList, aStorageNames));
}

bool SyncDBusAdaptor::removeProfile(const QString &aProfileId)
{
    // handle method call com.meego.msyncd.removeProfile
    bool out0;
    QMetaObject::invokeMethod(parent(), "removeProfile", Q_RETURN_ARG(bool, out0), Q_ARG(QString, aProfileId));
    return out0;
}

bool SyncDBusAdaptor::requestStorages(const QStringList &aStorageNames)
{
    // handle method call com.meego.msyncd.requestStorages
    bool out0;
    QMetaObject::invokeMethod(parent(), "requestStorages", Q_RETURN_ARG(bool, out0), Q_ARG(QStringList, aStorageNames));
    return out0;
}

QStringList SyncDBusAdaptor::runningSyncs()
{
    // handle method call com.meego.msyncd.runningSyncs
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "runningSyncs", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

bool SyncDBusAdaptor::saveSyncResults(const QString &aProfileId, const QString &aSyncResults)
{
    // handle method call com.meego.msyncd.saveSyncResults
    bool out0;
    QMetaObject::invokeMethod(parent(), "saveSyncResults", Q_RETURN_ARG(bool, out0), Q_ARG(QString, aProfileId), Q_ARG(QString, aSyncResults));
    return out0;
}

bool SyncDBusAdaptor::setSyncSchedule(const QString &aProfileId, const QString &aScheduleAsXml)
{
    // handle method call com.meego.msyncd.setSyncSchedule
    bool out0;
    QMetaObject::invokeMethod(parent(), "setSyncSchedule", Q_RETURN_ARG(bool, out0), Q_ARG(QString, aProfileId), Q_ARG(QString, aScheduleAsXml));
    return out0;
}

void SyncDBusAdaptor::start(uint aAccountId)
{
    // handle method call com.meego.msyncd.start
    QMetaObject::invokeMethod(parent(), "start", Q_ARG(uint, aAccountId));
}

bool SyncDBusAdaptor::startSync(const QString &aProfileId)
{
    // handle method call com.meego.msyncd.startSync
    bool out0;
    QMetaObject::invokeMethod(parent(), "startSync", Q_RETURN_ARG(bool, out0), Q_ARG(QString, aProfileId));
    return out0;
}

int SyncDBusAdaptor::status(uint aAccountId, int &aFailedReason, qlonglong &aPrevSyncTime, qlonglong &aNextSyncTime)
{
    // handle method call com.meego.msyncd.status
    return static_cast<Synchronizer *>(parent())->status(aAccountId, aFailedReason, aPrevSyncTime, aNextSyncTime);
}

void SyncDBusAdaptor::stop(uint aAccountId)
{
    // handle method call com.meego.msyncd.stop
    QMetaObject::invokeMethod(parent(), "stop", Q_ARG(uint, aAccountId));
}

QString SyncDBusAdaptor::syncProfile(const QString &aProfileId)
{
    // handle method call com.meego.msyncd.syncProfile
    QString out0;
    QMetaObject::invokeMethod(parent(), "syncProfile", Q_RETURN_ARG(QString, out0), Q_ARG(QString, aProfileId));
    return out0;
}

QStringList SyncDBusAdaptor::syncProfilesByKey(const QString &aKey, const QString &aValue)
{
    // handle method call com.meego.msyncd.syncProfilesByKey
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "syncProfilesByKey", Q_RETURN_ARG(QStringList, out0), Q_ARG(QString, aKey), Q_ARG(QString, aValue));
    return out0;
}

QStringList SyncDBusAdaptor::syncProfilesByType(const QString &aType)
{
    // handle method call com.meego.msyncd.syncProfilesByType
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "syncProfilesByType", Q_RETURN_ARG(QStringList, out0), Q_ARG(QString, aType));
    return out0;
}

QList<uint> SyncDBusAdaptor::syncingAccounts()
{
    // handle method call com.meego.msyncd.syncingAccounts
    QList<uint> out0;
    QMetaObject::invokeMethod(parent(), "syncingAccounts", Q_RETURN_ARG(QList<uint>, out0));
    return out0;
}

bool SyncDBusAdaptor::updateProfile(const QString &aProfileAsXml)
{
    // handle method call com.meego.msyncd.updateProfile
    bool out0;
    QMetaObject::invokeMethod(parent(), "updateProfile", Q_RETURN_ARG(bool, out0), Q_ARG(QString, aProfileAsXml));
    return out0;
}

