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

#include <QString>
#include <QDomDocument>
#include <libsyncprofile/SyncProfile.h>
#include <libsyncprofile/SyncResults.h>
#include <libsyncprofile/SyncSchedule.h>
#include "SyncClientInterfacePrivate.h"
#include "SyncClientInterface.h"
#include "LogMacros.h"

using namespace Buteo;

static const QString SYNC_DBUS_OBJECT = "/synchronizer";
static const QString SYNC_DBUS_SERVICE = "com.nokia.msyncd";

SyncClientInterfacePrivate::SyncClientInterfacePrivate(SyncClientInterface *aParent) :
			iParent(aParent) {
	iSyncDaemon = new SyncDaemonProxy(SYNC_DBUS_SERVICE, SYNC_DBUS_OBJECT,
			QDBusConnection::sessionBus(), this);
	if (iSyncDaemon) {
		connect(iSyncDaemon,SIGNAL(signalProfileChanged(QString,int,QString)),
				this,SLOT(profileChanged(QString,int,QString)));
		connect(iSyncDaemon, SIGNAL(resultsAvailable(QString, QString)), this,
				SLOT(resultsAvailable(QString, QString)));
		connect(this,SIGNAL(profileChanged(QString,int,Buteo::SyncProfile)),
				iParent,SIGNAL(profileChanged(QString,int,Buteo::SyncProfile)));
		connect(this,SIGNAL(resultsAvailable(QString,Buteo::SyncResults)),
				iParent,SIGNAL(resultsAvailable(QString,Buteo::SyncResults)));
		connect	(iSyncDaemon,SIGNAL(syncStatus(QString,int,QString,int)),
				iParent,SIGNAL(syncStatus(QString,int,QString,int)));
		connect(iSyncDaemon,SIGNAL(transferProgress(QString,int,int,QString,int)),
				iParent,SIGNAL(transferProgress(QString,int,int,QString,int)));
		connect(iSyncDaemon, SIGNAL(backupInProgress()),
				iParent, SIGNAL(backupInProgress()));
		connect(iSyncDaemon, SIGNAL(backupDone()),
				iParent, SIGNAL(backupDone()));
		connect(iSyncDaemon, SIGNAL(restoreInProgress()),
				iParent, SIGNAL(restoreInProgress()));
		connect(iSyncDaemon, SIGNAL(restoreDone()),
				iParent, SIGNAL(restoreDone()));
	}
	qRegisterMetaType<Buteo::Profile>("Buteo::Profile");
	qRegisterMetaType<Buteo::SyncResults>("Buteo::SyncResults");
}

SyncClientInterfacePrivate::~SyncClientInterfacePrivate()
{
	delete iSyncDaemon;
	iSyncDaemon = NULL;
}

bool SyncClientInterfacePrivate::startSync(const QString &aProfileId) const
{
	bool syncStatus = false;

	if (iSyncDaemon && !aProfileId.isEmpty()) {
		syncStatus = iSyncDaemon->startSync(aProfileId);
	}

	return syncStatus;
}

void SyncClientInterfacePrivate::abortSync(const QString &aProfileId) const
{
	if (iSyncDaemon && !aProfileId.isEmpty()) {
		iSyncDaemon->abortSync(aProfileId);
	}
}

QStringList SyncClientInterfacePrivate::getRunningSyncList()
{
	QStringList runningSyncList;
	if (iSyncDaemon) {
		runningSyncList = iSyncDaemon->runningSyncs();
	}
	return runningSyncList;
}

bool SyncClientInterfacePrivate::addProfile(Buteo::SyncProfile &aProfile)
{
	bool status = false;
	if (iSyncDaemon) {
		QString profileAsXmlString = aProfile.toString();
		status = iSyncDaemon->addProfile(profileAsXmlString);
	}
	return status;
}

bool SyncClientInterfacePrivate::removeProfile(QString &aProfileId)
{
	bool status = false;
	if (iSyncDaemon) {
		status = iSyncDaemon->removeProfile(aProfileId);
	}
	return status;
}

bool SyncClientInterfacePrivate::updateProfile(Buteo::SyncProfile &aProfile)
{
	bool status = false;
	if (iSyncDaemon) {
		QString profileAsXmlString = aProfile.toString();
		status = iSyncDaemon->updateProfile(profileAsXmlString);
	}
	return status;
}

void SyncClientInterfacePrivate::profileChanged(QString aProfileId,int aChangeType,
		QString aProfileAsXml)
{
	QDomDocument doc;
	if (doc.setContent(aProfileAsXml, true)) {
		Buteo::SyncProfile profile(doc.documentElement());
		emit profileChanged(aProfileId,aChangeType,profile);
	} else {
		LOG_DEBUG("Invalid Profile Xml Received from msyncd");
	}
}

void SyncClientInterfacePrivate::resultsAvailable(QString aProfileId,
		QString aLastResultsAsXml)
{
	QDomDocument doc;
	if (doc.setContent(aLastResultsAsXml, true)) {
		Buteo::SyncResults results(doc.documentElement());
		emit resultsAvailable(aProfileId, results);
	} else {
		LOG_DEBUG("Invalid Profile Xml Received from msyncd");
	}
}

bool SyncClientInterfacePrivate::setSyncSchedule(QString &aProfileId,
		SyncSchedule &aSchedule)
{
	bool status = false;
	if (iSyncDaemon) {
		QString scheduleAsXmlString = aSchedule.toString();
		if (!scheduleAsXmlString.isEmpty()) {
			status = iSyncDaemon->setSyncSchedule(aProfileId,
					scheduleAsXmlString);
		}
	}
	return status;
}

bool SyncClientInterfacePrivate::saveSyncResults(const QString &aProfileId,
        const Buteo::SyncResults &aSyncResults)
{
    bool status = false;
    if (iSyncDaemon) {
        QString resultsAsXmlString = aSyncResults.toString();
        if (!resultsAsXmlString.isEmpty()) {
            status = iSyncDaemon->saveSyncResults(aProfileId, resultsAsXmlString);
        }
    }
    return status;
}

bool  SyncClientInterfacePrivate::getBackUpRestoreState()
{
	bool status = false;
	if (iSyncDaemon) {
		status = iSyncDaemon->getBackUpRestoreState();
	}
	return status;
}

bool SyncClientInterfacePrivate::isValid()
{
	return(iSyncDaemon && iSyncDaemon->isValid());
}

Buteo::SyncResults SyncClientInterfacePrivate::getLastSyncResult(const QString &aProfileId)
{
    Buteo::SyncResults syncResult(QDateTime::currentDateTime(),
         SyncResults::SYNC_RESULT_INVALID, Buteo::SyncResults::SYNC_RESULT_INVALID);

    if (iSyncDaemon) {
        QString resultASXmlString = iSyncDaemon->getLastSyncResult(aProfileId);
        QDomDocument doc;

        if (doc.setContent(resultASXmlString, true)) {
            Buteo::SyncResults result(doc.documentElement());
            return result;
        }
        else {
            LOG_CRITICAL("Invalid Profile Xml Received from msyncd");
        }
    }
    return syncResult;
}
