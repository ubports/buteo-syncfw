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
#ifndef PLUGINRUNNERTEST_H_
#define PLUGINRUNNERTEST_H_

#include <QtTest/QtTest>
#include <QString>

#include "PluginRunner.h"
#include "PluginManager.h"
#include "SyncProfile.h"
#include "ClientPluginRunner.h"

namespace Buteo {
    
class PluginRunnerTest : public QObject
{
	Q_OBJECT
	
	private slots:
	void initTestCase();
	void testPluginRunnerConstructor();
	void cleanupTestCase();

	private:
	PluginRunner *iPRunner;
	PluginManager *iPManager;
	SyncProfile *iSProfile;
	ClientPluginRunner *iClientPRunner;
};

}
#endif /*PLUGINRUNNERTEST_H_*/
