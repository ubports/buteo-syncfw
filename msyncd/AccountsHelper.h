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
    
class AccountsHelper : public QObject
{
    Q_OBJECT

public:
    AccountsHelper(ProfileManager &aProfileManager, QObject *aParent = NULL);

    virtual ~AccountsHelper();

public Q_SLOTS:

    void slotAccountCreated(Accounts::AccountId id);

    void slotAccountRemoved(Accounts::AccountId id);

    void slotAccountEnabled(Accounts::AccountId id);

    void slotAccountNameChanged(const QString &newName);

Q_SIGNALS:
    /*! \brief The signal is emitted when a profile gets added, removed, or modified
     *
     * \param profileName The name of the profile that changed
     * \param changeType
     *          0 -- Profile added
     *          1 -- Profile modified
     *          2 -- Profile deleted
     */
    void profileChanged(QString profileName, int changeType);
private:

    QList<SyncProfile*> getProfilesByAccountId(Accounts::AccountId);

    void addAccountIfNotExists(const Accounts::Account *account,
                               const Accounts::Service *service,
                               const SyncProfile *baseProfile);

    void registerAccountListeners();

    void registerAccountListener(Accounts::AccountId id);

    Accounts::Manager *iAccountManager;

    ProfileManager &iProfileManager;

    QList<Accounts::Account*> iAccountList;

    friend class AccountsHelperTest;
};

}

#endif // ACCOUNTSHELPER_H
