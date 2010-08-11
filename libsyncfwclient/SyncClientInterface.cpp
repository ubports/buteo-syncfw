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

#include "SyncClientInterface.h"
#include "SyncClientInterfacePrivate.h"

using namespace Buteo;

SyncClientInterface::SyncClientInterface():
		d_ptr(new SyncClientInterfacePrivate(this))
{

}

SyncClientInterface::~SyncClientInterface()
{
	delete d_ptr;
	d_ptr = NULL;
}

bool SyncClientInterface::startSync(const QString &aProfileId) const
{
	return d_ptr->startSync(aProfileId);
}

void SyncClientInterface::abortSync(const QString &aProfileId) const
{
	d_ptr->abortSync(aProfileId);
}

QStringList SyncClientInterface::getRunningSyncList()
{
	return d_ptr->getRunningSyncList();
}

bool SyncClientInterface::addProfile(Buteo::SyncProfile &aProfile)
{
	return d_ptr->addProfile(aProfile);
}

bool SyncClientInterface::removeProfile(QString &aProfileId)
{
	return d_ptr->removeProfile(aProfileId);
}

bool SyncClientInterface::updateProfile(Buteo::SyncProfile &aProfile)
{
	return d_ptr->updateProfile(aProfile);
}

bool SyncClientInterface::setSyncSchedule(QString &aProfileId,SyncSchedule &aSchedule)
{
	return d_ptr->setSyncSchedule(aProfileId,aSchedule);
}

bool SyncClientInterface::saveSyncResults(const QString &aProfileId,const Buteo::SyncResults &aSyncResults)
{
    return d_ptr->saveSyncResults(aProfileId,aSyncResults);
}

bool SyncClientInterface::getBackUpRestoreState()
{
	return d_ptr->getBackUpRestoreState();
}

bool SyncClientInterface::isValid()
{
	return d_ptr->isValid();
}

void SyncClientInterface::lastSyncResultCode(const QString &aProfileId, int &majorCode, int &minorCode)
{
    d_ptr->lastSyncResultCode(aProfileId, majorCode, minorCode);
}

QDateTime SyncClientInterface::lastSyncTime(const QString &aProfileId)
{
    return d_ptr->lastSyncTime(aProfileId);
}

bool SyncClientInterface::isLastSyncScheduled(const QString &aProfileId) const
{
    return d_ptr->isLastSyncScheduled(aProfileId);
}
