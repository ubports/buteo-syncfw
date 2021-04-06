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
#include "AccountsHelperTest.h"
#include <Profile.h>
#include <ProfileEngineDefs.h>

using namespace Buteo;

static const QString PROFILE_XML =
    "<profile name=\"ovi-sync\" type=\"sync\">"
    "<profile name=\"ovi\" type=\"service\">"
    "</profile>"
    "<key name=\"enabled\" value=\"true\" />"
    "</profile>";

static const QString OVI_PROVIDER = "ovi";
static const QString DUMMY_USER = "dummy";
static const QString USERPROFILE_DIR = "syncprofiletests/testprofiles/user";
static const QString SYSTEMPROFILE_DIR = "syncprofiletests/testprofiles/system";
static const QString SERVICE_SYNC = "Sync";
static const QString SERVICE_NAME = "testsync-ovi";

AccountsHelperTest::AccountsHelperTest()
    :   QObject(NULL),
        iManager(SERVICE_SYNC, this),
        iProfileManager(USERPROFILE_DIR, SYSTEMPROFILE_DIR)
{
    iAccountsHelper = new AccountsHelper(iProfileManager, 0);
}

void AccountsHelperTest::initTestCase()
{
    iAccount = iManager.createAccount(OVI_PROVIDER);
    iAccount->setDisplayName(DUMMY_USER);
    iAccount->setEnabled(true);
    Accounts::Service service = iManager.service(SERVICE_NAME);
    iAccount->selectService(service);
    iAccount->setEnabled(true);
    iAccount->selectService();
    QVERIFY(iAccount != NULL);
    iAccount->sync();
}

void AccountsHelperTest::cleanupTestCase()
{
    if (iAccount != NULL) {
        iAccount->remove();
        iAccount->sync();
        delete iAccount;
        iAccount = NULL;
    } // no else
}

void AccountsHelperTest::testProfileAdded()
{
    // Ensure that the profile with the username was added correctly
    SyncProfile *syncProfile = iProfileManager.syncProfile(SERVICE_NAME + "-" +
                                                           iAccount->displayName());
    //QVERIFY(syncProfile != 0);
    Q_UNUSED(syncProfile);
}

void AccountsHelperTest::testAddAccountData()
{

}

QTEST_MAIN(Buteo::AccountsHelperTest)
