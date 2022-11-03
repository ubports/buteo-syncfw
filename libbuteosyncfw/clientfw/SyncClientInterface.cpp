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

Buteo::SyncResults SyncClientInterface::getLastSyncResult(const QString &aProfileId)
{
    return d_ptr->getLastSyncResult(aProfileId);
}


QList<QString /*profileAsXml*/> SyncClientInterface::allVisibleSyncProfiles()
{
    return d_ptr->allVisibleSyncProfiles();
}


QString SyncClientInterface::syncProfile(const QString &aProfileId)
{
    return d_ptr->syncProfile(aProfileId);
}

QStringList SyncClientInterface::syncProfilesByKey(const QString &aKey, const QString &aValue)
{
    return d_ptr->syncProfilesByKey(aKey, aValue);
}

QStringList SyncClientInterface::syncProfilesByType(const QString &aType)
{
    return d_ptr->syncProfilesByType(aType);
}
