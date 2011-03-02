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
#ifndef ACCOUNTSHELPER_H
#define ACCOUNTSHELPER_H

#include <QObject>

#include <accounts-qt/manager.h>
#include <accounts-qt/account.h>

namespace Buteo {
                 
class Profile;
class AccountsHelperTest;
class ProfileManager;
class SyncProfile;
    
/*! \brief Helper Class towards Accounts::Manager and various SSO related
 *  operations.
 */
class AccountsHelper : public QObject
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * \param aProfileManager - reference to Profile Manager Object
     * \param aParent - Parent object
     */
    AccountsHelper(ProfileManager &aProfileManager, QObject *aParent = NULL);

    /*! \brief Destructor
     *
     */
    virtual ~AccountsHelper();

public Q_SLOTS:

	/*! \brief slot for Accounts::Manager accountCreated signal
	 *
	 * \param id Accounts Id
	 */
    void slotAccountCreated(Accounts::AccountId id);

	/*! \brief slot for Accounts::Manager accountRemoved signal
	 *
	 * \param id of the accounts
	 */
    void slotAccountRemoved(Accounts::AccountId id);

	/*! \brief slot for Accounts::Manager accountEnabled signal
	 *
	 * \param id of the accounts
	 */
    void slotAccountEnabled(Accounts::AccountId id);

	/*! \brief slot for Accounts::Manager displayNameChanged signal
	 * *
	 * \param newName of the accounts
	 */
    void slotAccountNameChanged(const QString &newName);
    
	/*! \brief slot for Accounts::Manager displayNameChanged signal
	 * *
	 * \param id of the accounts
	 */
    void slotAccountUpdated(Accounts::AccountId id);

Q_SIGNALS:

    void enableSOC(const QString& aProfileName);
    void removeProfile(QString profileId);

private:

    QList<SyncProfile*> getProfilesByAccountId(Accounts::AccountId);

    void addAccountIfNotExists(const Accounts::Account *account,
                               const Accounts::Service *service,
                               const SyncProfile *baseProfile);

    void setSyncSchedule(SyncProfile *syncProfile, Accounts::AccountId id, bool aCreateNew = false);
    void registerAccountListeners();

    void registerAccountListener(Accounts::AccountId id);

    Accounts::Manager *iAccountManager;

    ProfileManager &iProfileManager;

    QList<Accounts::Account*> iAccountList;

#ifdef SYNCFW_UNIT_TESTS
    friend class AccountsHelperTest;
#endif
};

}

#endif // ACCOUNTSHELPER_H
