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
#ifndef SYNCHRONIZERTEST_H_
#define SYNCHRONIZERTEST_H_

#include <QtTest/QtTest>
#include <QString>
#include <QStringList>

#include "synchronizer.h"
#include "SyncSession.h"

namespace Buteo {
    
class SynchronizerTest: public QObject
{
	Q_OBJECT
	private slots:
	void initTestCase();
	void cleanupTestCase();
	void testSyncConstructor();
	void testInitialize();
	void testSync();
	void testSignals();
	
	private:
	Synchronizer *iSync;
	SyncSession *iSyncSession;
	SyncProfile *iProfile;
	SyncSession *iSyncSessionQ;
	SyncProfile *iProfileQ;
};

}
#endif /*SYNCHRONIZERTEST_H_*/
