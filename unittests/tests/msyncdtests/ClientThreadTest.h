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
#ifndef CLIENTTHREADTEST_H_
#define CLIENTTHREADTEST_H_

#include <QtTest/QtTest>
#include <QString>

#include <SyncCommonDefs.h>
#include "ClientThread.h"
#include "ClientPlugin.h"
#include "SyncProfile.h"

namespace Buteo {
    
class ClientPluginDerived: public ClientPlugin
{
	Q_OBJECT

	public:
	ClientPluginDerived(const QString& aPluginName,
                  	    const SyncProfile& aProfile,
                  	    PluginCbInterface* aCbInterface);
	bool startSync();
	bool init();
	bool uninit();
	virtual bool cleanUp();

	bool iTestClSignal;

	public slots:
	void connectivityStateChanged(Sync::ConnectivityType aType, bool aState);
	
};
class ClientThreadTest: public QObject
{
	Q_OBJECT

	private slots:
	void initTestCase();
	void cleanupTestCase();
	void testClientThreadConstructor();
	void testGetPlugin();
	void testGetProfileName();
	void testClientThread();
	void testGetSyncResults();
	void testInitError();

	private:
	ClientThread *iClientThread;
	ClientPluginDerived *iPluginDerived;
	ClientPlugin *iPlugin;
	SyncProfile *iSyncProfile;
	bool iClientThreadRet;
};

}
#endif /*CLIENTTHREADTEST_H_*/
