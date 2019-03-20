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
#ifndef SERVERTHREADTEST_H_
#define SERVERTHREADTEST_H_

#include <QtTest/QtTest>
#include <QString>
#include <QThread>
#include <QSignalSpy>

#include "ServerThread.h"
#include "ServerPlugin.h"
#include "SyncCommonDefs.h"
#include "Profile.h"

namespace Buteo {
    
class ServerPluginDerived: public ServerPlugin
{
	Q_OBJECT

	public:
	ServerPluginDerived(const QString& aPluginName,
			    const Profile& aProfile,
			    PluginCbInterface* aCbInterface);

	bool startListen();
	void stopListen();
	bool init();
	bool uninit();
        void suspend();
        void resume();
	virtual bool cleanUp();

	bool iTestSignal;
	
	public slots:
	void connectivityStateChanged( Sync::ConnectivityType , bool);
};

class ServerThreadTest: public QObject
{
	Q_OBJECT

	private slots:	
	void initTestCase();
	void cleanupTestCase();
	void testServerThreadConstructor();
	void testGetPlugin();	
	void testGetProfileName();
	void testThread();
	void testStopThErrorSignal();
	
	private:
	ServerThread *iServerThread;
	ServerPluginDerived *iPluginDerived;
	ServerPlugin *iThreadTestSp;	
	const Profile *iProfile;
	bool iThreadreturn;
};

}

#endif /*SERVERTHREADTEST_H_*/

