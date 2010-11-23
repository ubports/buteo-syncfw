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
#include "AccountsHelper.h"
#include "LogMacros.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "ProfileEngineDefs.h"

using namespace Buteo;

AccountsHelper::AccountsHelper(ProfileManager &aProfileManager, QObject *aParent)
:   QObject(aParent), iProfileManager(aProfileManager)
{
    iAccountManager = new Accounts::Manager(this);
    // Connect to signal for account creation, deletion, and modification
    QObject::connect(iAccountManager, SIGNAL(accountCreated(Accounts::AccountId)),
                     this, SLOT(slotAccountCreated(Accounts::AccountId)));
    QObject::connect(iAccountManager, SIGNAL(accountRemoved(Accounts::AccountId)),
                     this, SLOT(slotAccountRemoved(Accounts::AccountId)));
    QObject::connect(iAccountManager, SIGNAL(enabledEvent(Accounts::AccountId)),
                     this, SLOT(slotAccountEnabled(Accounts::AccountId)));
    QObject::connect(iAccountManager, SIGNAL(accountUpdated(Accounts::AccountId)),
                     this, SLOT(slotAccountUpdated(Accounts::AccountId)));

    registerAccountListeners();
}

AccountsHelper::~AccountsHelper()
{
    delete iAccountManager;
    iAccountManager = 0;
    // There is no need to delete the accounts objects as they get deleted by
    // their parent, aka, the manager
}

void AccountsHelper::slotAccountCreated(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    Accounts::Account *newAccount = iAccountManager->account(id);
    if(0 != newAccount)
    {
        registerAccountListener(id);
        // We will get only sync services.
        Accounts::ServiceList serviceList = newAccount->enabledServices();
        foreach(Accounts::Service *service, serviceList)
        {
            // Look for a sync profile that matches the service name
            LOG_DEBUG("Looking for sync profile::" << service->name());
            SyncProfile *syncProfile = iProfileManager.syncProfile(service->name());
            LOG_DEBUG("Found profile::" << service->name());
            if(0 != syncProfile &&
               (true == syncProfile->boolKey(KEY_USE_ACCOUNTS, false))
               )
            {
                addAccountIfNotExists(newAccount, service, syncProfile);
                delete syncProfile;
                syncProfile = 0;
            }
        }
        delete newAccount;
    }
}

void AccountsHelper::slotAccountRemoved(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    // Delete the profile(s) with account ID
    QList<SyncProfile*> syncProfiles = getProfilesByAccountId(id);
    foreach(SyncProfile *profile, syncProfiles)
    {
        LOG_DEBUG("Removing profile" << profile->name());
        // Delete the profile
        iProfileManager.removeProfile(profile->name());

        delete profile;
    }
}

void AccountsHelper::slotAccountEnabled(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    Accounts::Account *account = iAccountManager->account(id);
    if(0 != account)
    {
        // Get all enabled services for this account
        Accounts::ServiceList serviceList = account->enabledServices();
        // Get all profiles for this account id
        QList<SyncProfile*> profiles = getProfilesByAccountId(id);
        // Check if profiles are already configured for the account
        if(false == account->enabled())
        {
            foreach(SyncProfile *profile, profiles)
            {
                profile->setKey(KEY_HIDDEN, BOOLEAN_TRUE);
                iProfileManager.updateProfile(*profile);
                delete profile;
            }
        }
        else
        {
            foreach(Accounts::Service *service, serviceList)
            {
                SyncProfile *syncProfile = iProfileManager.syncProfile(service->name());
                if(0 != syncProfile)
                {
                    addAccountIfNotExists(account, service, syncProfile);
                    delete syncProfile;
                    syncProfile = 0;
                }
                for(QList<SyncProfile*>::Iterator i = profiles.begin(); i != profiles.end(); )
                {
                    // If the service name is still enabled, remove the sync profile
                    // from the list
                    if((*i)->name().startsWith(service->name()))
                    {
                        delete *i;
                        i = profiles.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }

            // Now disable all remaining profiles
            foreach(SyncProfile *profile, profiles)
            {
                profile->setKey(KEY_ACTIVE, BOOLEAN_FALSE);
                iProfileManager.updateProfile(*profile);
                delete profile;
            }
        }
    }
}

void AccountsHelper::slotAccountNameChanged(const QString &newName)
{
    FUNCTION_CALL_TRACE;
    // Get the sender account
    Accounts::Account *changedAccount = qobject_cast<Accounts::Account*>(this->sender());
    if(0 != changedAccount)
    {
        QString newProfileName;
        // Rename all the sync profiles associated with this account id
        QList<SyncProfile*> syncProfiles = getProfilesByAccountId(changedAccount->id());
        foreach(SyncProfile *syncProfile, syncProfiles)
        {
            QString profileName = syncProfile->name();
            newProfileName = profileName.mid(0, profileName.lastIndexOf('-')) +
                             "-" + newName;
            LOG_INFO("Renaming profile" << profileName << "to" << newProfileName);
            iProfileManager.rename(profileName, newName);
            delete syncProfile;
        }
    }
}

void AccountsHelper::setSyncSchedule(SyncProfile *syncProfile, Accounts::AccountId id, bool aCreateNew)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (aCreateNew);
    Accounts::Account *account = iAccountManager->account(id);
    if(0 != account) {
        //Sync schedule settings should be global
        account->selectService();
        SyncSchedule syncSchedule;

        int peakStart = account->valueAsInt(Buteo::SYNC_SCHEDULE_PEAK_START_TIME_KEY_INT);
        QTime startTime(peakStart/60 , peakStart%60);

        int peakEnd = account->valueAsInt(Buteo::SYNC_SCHEDULE_PEAK_END_TIME_KEY_INT);
        QTime endTime(peakEnd/60 , peakEnd%60);
        LOG_DEBUG ("Start time:" << startTime << "End Time :" << endTime);
        syncSchedule.setRushTime(startTime, endTime);

        SyncProfile::SyncType syncType = account->valueAsBool (Buteo::SYNC_SCHEDULE_ENABLED_KEY_BOOL) ? SyncProfile::SYNC_SCHEDULED:SyncProfile::SYNC_MANUAL ;
        syncProfile->setSyncType (syncType);
        LOG_DEBUG ("Sync Type :" << syncType );

        syncSchedule.setInterval (account->valueAsInt (Buteo::SYNC_SCHEDULE_OFFPEAK_SCHEDULE_KEY_INT));
        LOG_DEBUG ("Sync Interval :" << account->valueAsInt (Buteo::SYNC_SCHEDULE_OFFPEAK_SCHEDULE_KEY_INT));
        if (syncSchedule.interval() == 0)
            syncSchedule.setRushEnabled (false);

        syncSchedule.setRushInterval (account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_SCHEDULE_KEY_INT));
        LOG_DEBUG ("Sync Rush Interval :" << account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_SCHEDULE_KEY_INT));

        int map = account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_DAYS_KEY_INT);
        LOG_DEBUG ("Sync Days :" << account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_DAYS_KEY_INT));
        Buteo::DaySet rdays;
        Buteo::DaySet days;
        int lastDay = Qt::Sunday;
        while (lastDay > 0) {
            int val = 0;
            val |= (1 << lastDay);
            days.insert(lastDay);
            if ((val & map)) {
                days.insert(lastDay);
                LOG_DEBUG ("Day :" << lastDay);
            }
            --lastDay;
        }
        syncSchedule.setRushDays(rdays);
        syncSchedule.setDays(days);
        syncProfile->setSyncSchedule (syncSchedule);
    }
}

void AccountsHelper::slotAccountUpdated(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    QList<SyncProfile*> syncProfiles = getProfilesByAccountId(id);
    foreach(SyncProfile *syncProfile, syncProfiles)
    {
        setSyncSchedule(syncProfile, id);
        iProfileManager.updateProfile(*syncProfile);
        delete syncProfile;
    }
}

QList<SyncProfile*> AccountsHelper::getProfilesByAccountId(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    QList<ProfileManager::SearchCriteria> filters;
    ProfileManager::SearchCriteria filter;
    filter.iType = ProfileManager::SearchCriteria::EQUAL;
    filter.iKey = KEY_ACCOUNT_ID;
    filter.iValue = QString::number(id);
    filters.append(filter);
    return iProfileManager.getSyncProfilesByData(filters);
}

void AccountsHelper::addAccountIfNotExists(const Accounts::Account *account,
                                           const Accounts::Service *service,
                                           const SyncProfile *baseProfile)
{
    FUNCTION_CALL_TRACE;

    Profile *serviceProfile = iProfileManager.profile(service->name(), Profile::TYPE_SERVICE);
    if (!serviceProfile) {
    	LOG_DEBUG ("!!!! Service not supported !!!!");
	return;
    }
    
    QString profileName ;
    if (!service->name().isEmpty()) {
	    QStringList keys;
	    keys << QString::number(account->id()) << service->name();
	    serviceProfile->setName(keys);
            profileName = serviceProfile->name();
    }
    delete serviceProfile;
   
    SyncProfile *profile = iProfileManager.syncProfile(profileName);
    
    if(0 == profile)
    {
        // Create a new sync profile with username
        SyncProfile *newProfile = baseProfile->clone();
        newProfile->setName(profileName);
        newProfile->setKey(KEY_DISPLAY_NAME, service->name() + "-" + account->displayName());
        // Add the account ID to the profile
        newProfile->setKey(KEY_ACCOUNT_ID, QString::number(account->id()));
        // Set profile as enabled
        newProfile->setKey(KEY_HIDDEN, BOOLEAN_FALSE);
        newProfile->setKey(KEY_ACTIVE, BOOLEAN_TRUE);
	setSyncSchedule (newProfile, account->id(), true);
        // Save the newly created profile
        iProfileManager.updateProfile(*newProfile);
        delete newProfile;
    }
    else if(true == profile->boolKey(KEY_USE_ACCOUNTS, false))
    {
        // Set profile as enabled
        profile->setKey(KEY_ACTIVE, BOOLEAN_FALSE);
        profile->setKey(KEY_HIDDEN, BOOLEAN_FALSE);
        iProfileManager.updateProfile(*profile);
        delete profile;
    }
}

void AccountsHelper::registerAccountListeners()
{
    FUNCTION_CALL_TRACE;
    // Populate all enabled accounts list (so that we can listen to changes)
    QList<Accounts::AccountId> accountIds = iAccountManager->accountList();
    foreach(Accounts::AccountId id, accountIds)
    {
        registerAccountListener(id);
    }
}

void AccountsHelper::registerAccountListener(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    Accounts::Account *account = iAccountManager->account(id);
    iAccountList.append(account);
    // Register callback for account name changed
    QObject::connect(account, SIGNAL(displayNameChanged(const QString&)),
                     this, SLOT(slotAccountNameChanged(const QString&)));
}
