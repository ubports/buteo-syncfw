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
#ifndef SYNCPLUGINBASE_H
#define SYNCPLUGINBASE_H

#include "SyncCommonDefs.h"
#include "SyncResults.h"
#include <QString>
#include <QMetaType>
#include <QObject>
#include <QMap>



namespace Buteo {

class PluginCbInterface;

/*! \brief Base class for client and server plugins.
 *
 */
class SyncPluginBase : public QObject
{
	Q_OBJECT;

public:
	/*! \brief Constructor
	 *
	 * @param aPluginName Name of this plugin
	 * @param aProfileName Profile name
	 * @param aCbInterface Pointer to the callback interface
	 */
	SyncPluginBase( const QString &aPluginName,
			const QString &aProfileName,
			PluginCbInterface *aCbInterface );

	/*! \brief Returns the name of this plugin
	 *
	 * @return Name of the plugin
	 */
	QString getPluginName() const;

	/*! \brief Returns profile name
	 *
	 * @return Profile
	 */
	QString getProfileName() const;

	/*! \brief Initializes the plugin.
	 *
	 * It is recommended that the plugin should do not do any thread insecure
	 * initializations inside constructor, instead it should be done inside
	 * this method.
	 *
	 * @return True on success, otherwise false
	 */
	virtual bool init() = 0;

	/*! \brief Uninitializes the plugin
	 *
	 * @return True on success, otherwise false
	 */
	virtual bool uninit() = 0;

	/*! \brief Aborts synchronization
	 *
	 * Derived plug-in should implement this function and abort the sync
	 * session that is in progress when this function is called. A final signal
	 * (success or error) is still expected from the aborted session before
	 * it terminates.
	 */
    virtual void abortSync(Sync::SyncStatus aStatus = Sync::SYNC_ABORTED) { };

	/*! \brief Cleans up any sync related stuff (e.g sync anchors etc) when the
	 * profile is deleted
	 *
	 * Derived plug-in should implement this function and perform any cleanup
	 * operations if required when the profile is deleted
	 */
	virtual bool cleanUp() { return false; };

	/*! \brief Gets the results of the last completed sync session.
	 *
	 * This function should be called only after the sync session has finished,
	 * after an error or success signal has been emitted.
	 * The default implementation returns empty results, so derived plug-in
	 * should implement this function.
	 * @returns Sync results.
	 */
	virtual SyncResults getSyncResults() const;

signals:

	/*! \brief Emitted when progress has been made in synchronization in
	 * transferring items between local and remote database.
	 *
	 * @param aProfileName Name of the profile being synchronized
	 * @param aDatabase Indicates if progress has been made to local or remote database
	 * @param aType Type of progress made (item added, modified or deleted)
	 * @param aMimeType Mime type of the processed item
	 * @param aCommittedItems No. of items committed for this operation
	 */
	void transferProgress( const QString &aProfileName, Sync::TransferDatabase aDatabase,
			Sync::TransferType aType, const QString &aMimeType, int aCommittedItems );

	/*! \brief Emitted when error has occurred in synchronization and it
	 *         cannot be continued.
	 *
	 * @param aProfileName Name of the profile being synchronized
	 * @param aMessage Message data related to error event
	 * @param aErrorCode Error code
	 */
	void error( const QString &aProfileName, const QString &aMessage, int aErrorCode );

	/*! \brief Emitted when synchronization has been finished successfully.
	 *
	 * @param aProfileName Name of the profile being synchronized
	 * @param aMessage Message data related to finish event
	 */
	void success( const QString &aProfileName, const QString &aMessage );

	/*! \brief Emitted when a storage is requested and accquired.
	 *
	 * @param aMimeType Mime type of the processed item
	 */
	void accquiredStorage( const QString &aMimeType );

	/*! \brief Emitted during Sync Progress to indicate the detail of the current ongoing sync
	 *
	 * @param aProfileName Profile Name
	 * @param aProgressDetail Progress in Detail \see Sync::SyncProgressDetail
	 */
	void syncProgressDetail( const QString &aProfileName, int aProgressDetail);


public slots:

	/*! \brief Slot that is invoked by sync framework when changes occur in
	 *         connectivity domains
	 *
	 * @param aType Connectivity domain
	 * @param aState True if connectivity domain is now available, otherwise false
	 */
	virtual void connectivityStateChanged( Sync::ConnectivityType aType, bool aState ) = 0;

protected:

	//! Pointer to synchronizer
	PluginCbInterface*  iCbInterface;

        struct ReceivedItemDetails
        {
            int added;
            int deleted;
            int modified;
            int error;
            QString mime;
        };

        QMap<QString, ReceivedItemDetails> receivedItems;

private:

	QString iPluginName;

	QString iProfileName;
};

}

#endif // SYNCPLUGINBASE_H
