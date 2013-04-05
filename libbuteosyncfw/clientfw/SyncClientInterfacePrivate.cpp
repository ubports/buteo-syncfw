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
#include <libsyncprofile/ProfileManager.h>
#include <libsyncprofile/SyncProfile.h>
#include <libsyncprofile/SyncResults.h>
#include <libsyncprofile/SyncSchedule.h>
#include "SyncClientInterfacePrivate.h"
#include "SyncClientInterface.h"
#include "LogMacros.h"

using namespace Buteo;

static const QString SYNC_DBUS_OBJECT = "/synchronizer";
static const QString SYNC_DBUS_SERVICE = "com.meego.msyncd";

SyncClientInterfacePrivate::SyncClientInterfacePrivate(SyncClientInterface *aParent) :
            iParent(aParent)
{
    FUNCTION_CALL_TRACE;
	iSyncDaemon = new SyncDaemonProxy(SYNC_DBUS_SERVICE, SYNC_DBUS_OBJECT,
			QDBusConnection::sessionBus(), this);
	if (iSyncDaemon) {
		connect(iSyncDaemon,SIGNAL(signalProfileChanged(QString,int,QString)),
                this,SLOT(slotProfileChanged(QString,int,QString)));

		connect(iSyncDaemon, SIGNAL(resultsAvailable(QString, QString)), this,
				SLOT(resultsAvailable(QString, QString)));

        connect(this,SIGNAL(profileChanged(QString, int, QString)),
                iParent,SIGNAL(profileChanged(QString, int, QString)));

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
    FUNCTION_CALL_TRACE;
	delete iSyncDaemon;
	iSyncDaemon = NULL;
}

bool SyncClientInterfacePrivate::startSync(const QString &aProfileId) const
{
    FUNCTION_CALL_TRACE;
	bool syncStatus = false;

	if (iSyncDaemon && !aProfileId.isEmpty()) {
		syncStatus = iSyncDaemon->startSync(aProfileId);
	}

	return syncStatus;
}

void SyncClientInterfacePrivate::abortSync(const QString &aProfileId) const
{
    FUNCTION_CALL_TRACE;
	if (iSyncDaemon && !aProfileId.isEmpty()) {
		iSyncDaemon->abortSync(aProfileId);
	}
}

QStringList SyncClientInterfacePrivate::getRunningSyncList()
{
    FUNCTION_CALL_TRACE;
	QStringList runningSyncList;
	if (iSyncDaemon) {
		runningSyncList = iSyncDaemon->runningSyncs();
	}
	return runningSyncList;
}

bool SyncClientInterfacePrivate::removeProfile(QString &aProfileId)
{
    FUNCTION_CALL_TRACE;
	bool status = false;
	if (iSyncDaemon) {
		status = iSyncDaemon->removeProfile(aProfileId);
	}
	return status;
}

bool SyncClientInterfacePrivate::updateProfile(Buteo::SyncProfile &aProfile)
{
    FUNCTION_CALL_TRACE;
	bool status = false;
	if (iSyncDaemon) {
		QString profileAsXmlString = aProfile.toString();
		status = iSyncDaemon->updateProfile(profileAsXmlString);
	}
	return status;
}

void SyncClientInterfacePrivate::slotProfileChanged(QString aProfileId,int aChangeType,
		QString aProfileAsXml)
{
    FUNCTION_CALL_TRACE;
    emit profileChanged(aProfileId,aChangeType,aProfileAsXml);
}

void SyncClientInterfacePrivate::resultsAvailable(QString aProfileId,
		QString aLastResultsAsXml)
{
    FUNCTION_CALL_TRACE;
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
    FUNCTION_CALL_TRACE;
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
    FUNCTION_CALL_TRACE;
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
    FUNCTION_CALL_TRACE;
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
    FUNCTION_CALL_TRACE;
    // Default construct with invalid values
    // Using default constructor for QDateTime() creates "null" date.
    Buteo::SyncResults syncResult(QDateTime(),
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

QList<QString /*profilesAsXml*/> SyncClientInterfacePrivate::allVisibleSyncProfiles()
{
    FUNCTION_CALL_TRACE;
    QList <QString> profilesAsXml;
    if (iSyncDaemon) {
        QStringList profilesList = iSyncDaemon->allVisibleSyncProfiles();
        if (!profilesList.isEmpty()) {
            foreach(QString profileAsXml, profilesList) {
                profilesAsXml.append(profileAsXml);
            }
        }
    }
    LOG_DEBUG("allVisibleSyncProfiles "<<profilesAsXml);
    return profilesAsXml;
}


QString SyncClientInterfacePrivate::syncProfile(const QString &aProfileId)
{
    FUNCTION_CALL_TRACE;
    QString profileAsXml;
    
    if (iSyncDaemon) {
        profileAsXml = iSyncDaemon->syncProfile(aProfileId);
    }

    LOG_DEBUG("syncProfile "<<profileAsXml);
    return profileAsXml;
}

QStringList SyncClientInterfacePrivate::syncProfilesByKey(const QString &aKey, const QString &aValue)
{
    FUNCTION_CALL_TRACE;
    QStringList profileAsXml;
    
    if (iSyncDaemon) {
        profileAsXml = iSyncDaemon->syncProfilesByKey(aKey, aValue);
    }
    
    return profileAsXml;
}

QStringList SyncClientInterfacePrivate::syncProfilesByType(const QString &aType)
{
    FUNCTION_CALL_TRACE;
    QStringList profileIds;
    
    if (iSyncDaemon) {
        profileIds = iSyncDaemon->syncProfilesByType(aType);
    }
    
    return profileIds;
}
