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

#ifndef TRANSPORTTRACKER_H_
#define TRANSPORTTRACKER_H_

#include "SyncCommonDefs.h"
#include <QObject>
#include <QMap>
#include <QMutex>

class ContextProperty;

namespace Buteo {

//class USBModedProxy;

    
/*! \brief Class for tracking transport states
 *
 * USB state is tracked with HAL, BT and Internet states with Context Framework.
 */
class TransportTracker : public QObject
{
	Q_OBJECT

public:

	/*! \brief Constructor
	 *
	 * @param aParent Parent object
	 */
	TransportTracker(QObject *aParent = 0);

	//! \brief Destructor
	virtual ~TransportTracker();

	/*! \brief Checks the state of the given connectivity type
	 *
	 * @param aType Connectivity type
	 * @return True if available, false if not
	 */
	bool isConnectivityAvailable(Sync::ConnectivityType aType) const;

	// @todo: make private
	/*! \brief updates the state of the given connectivity type to input value
	 *
	 * @param aType Connectivity type
	 * @param aState Connectivity State
	 */
	void updateState(Sync::ConnectivityType aType, bool aState);

signals:

    /*! \brief Signal emitted when a connectivity state changes
     *
     * @param aType Connectivity type whose state has changed
     * @param aState New state. True if available, false if not.
     */
	void connectivityStateChanged(Sync::ConnectivityType aType, bool aState);

private slots:

	void onUsbStateChanged(bool aConnected);

    void onBtStateChanged();

    void onInternetStateChanged();

private:

    QMap<Sync::ConnectivityType, bool> iTransportStates;

    //USBModedProxy *iUSBProxy;

    ContextProperty *iBt;

    ContextProperty *iInternet;

    mutable QMutex iMutex;

#ifdef SYNCFW_UNIT_TESTS
    friend class TransportTrackerTest;
#endif

};

}

#endif /* TRANSPORTTRACKER_H_ */
