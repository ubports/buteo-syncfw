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

#include <QTimer>

static const QString ACCOUNTS_GLOBAL_SERVICE("global");

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
    QObject::connect(iAccountManager, SIGNAL(accountUpdated(Accounts::AccountId)),
                     this, SLOT(slotAccountUpdated(Accounts::AccountId)));

    // load accounts after return from contructor, to allow connection with class singals
    // that can be fired by 'registerAccountListeners' function
    QTimer::singleShot(0, this, SLOT(registerAccountListeners()));
}

AccountsHelper::~AccountsHelper()
{
    iAcctWatchMap.clear();
    delete iAccountManager;
    iAccountManager = 0;
    // There is no need to delete the accounts objects as they get deleted by
    // their parent, aka, the manager
}

void AccountsHelper::slotAccountCreated(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    Accounts::Account *newAccount = iAccountManager->account(id);
    bool profileFoundAndCreated = false;

    if(0 != newAccount)
    {
        registerAccountListener(id);
        Accounts::ServiceList serviceList = newAccount->services();
        foreach(Accounts::Service service, serviceList)
        {
            // Look for a sync profile that matches the service name
            LOG_DEBUG("Looking for sync profile::" << service.name());
            SyncProfile *syncProfile = iProfileManager.syncProfile(service.name());
            LOG_DEBUG("Found profile::" << service.name());
            if(0 != syncProfile &&
                    (true == syncProfile->boolKey(KEY_USE_ACCOUNTS, false))
                    )
            {
                addAccountIfNotExists(newAccount, service, syncProfile);
                profileFoundAndCreated = true;
            }
            if(0 != syncProfile)
            {
                delete syncProfile;
                syncProfile = 0;
            }
        }

        if (profileFoundAndCreated == false)
        {
            // Fetch the key "remote_service_name" from the account settings and
            // use it to create a profile
            QString profileName = newAccount->valueAsString(REMOTE_SERVICE_NAME);
            LOG_DEBUG("Profile name from account setting:" << profileName);

            SyncProfile *syncProfile = iProfileManager.syncProfile( profileName );
            if (syncProfile)
            {
                if (syncProfile->boolKey (KEY_USE_ACCOUNTS, false) == true)
                    createProfileForAccount (newAccount, profileName, syncProfile);

                delete syncProfile;
            }
        }
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
        emit removeProfile(profile->name());
        delete profile;
    }
    // remove corresponding watch from the Map
    QMap<Accounts::Watch*, Accounts::AccountId>::iterator i = iAcctWatchMap.begin();
    while (i != iAcctWatchMap.end()) {
        if (i.value() == id) {
            i = iAcctWatchMap.erase(i);
            break;
        } else
            ++i;
    }
}

void AccountsHelper::slotAccountEnabledChanged(const QString &serviceName, bool enabled)
{
    FUNCTION_CALL_TRACE;
    // Get the sender account
    Accounts::Account *changedAccount = qobject_cast<Accounts::Account*>(this->sender());
    if(0 != changedAccount)
    {
        LOG_DEBUG("Received account enabled changed signal" << serviceName << enabled << changedAccount->displayName());
        if(serviceName == ACCOUNTS_GLOBAL_SERVICE || serviceName.isEmpty())
        {
            LOG_DEBUG("Entire account state changed to " << enabled << changedAccount->displayName());
            // Entire account disabled/enabled
            QList<SyncProfile*> profiles = getProfilesByAccountId(changedAccount->id());
            foreach(SyncProfile *profile, profiles)
            {
                // Check if the status really changed here
                // saving the account can trigger the emition of enabledChanged()
                if (profile->isEnabled() != enabled)
                {
                    LOG_DEBUG("Changing profile enabled" << profile->name() << enabled);
                    if(enabled)
                    {
                        // The service pointer must not be deleted here. It is
                        // cached by the Account manager object
                        Accounts::Service service = iAccountManager->service(profile->name());
                        changedAccount->selectService(service);
                        bool serviceEnabled = changedAccount->enabled();
                        changedAccount->selectService();
                        addSetting(changedAccount->id(), KEY_PROFILE_ID, QVariant(profile->name()));
                        LOG_DEBUG("Enabled status for service ::" << profile->name() << serviceEnabled);
                        if(serviceEnabled)
                        {
                            profile->setEnabled(changedAccount->enabled());
                            iProfileManager.updateProfile(*profile);
                            emit scheduleUpdated(profile->name());
                        }
                    }
                    else
                    {
                        // Unconditionally, set the profile as disabled
                        profile->setEnabled(enabled);
                        iProfileManager.updateProfile(*profile);
                        emit removeScheduledSync(profile->name());
                    }
                }
                delete profile;
            }
        }
        else
        {
            // If the account is in disabled state that will take precedence over the changed value
            // since those signals can come later
            if (!changedAccount->enabled()) {
                LOG_DEBUG("Account in disabled state, profile will be also set as disabled " << changedAccount->displayName());
                enabled = false;
            }
            // A service in the account is enabled/disabled
            QList<SyncProfile*> profiles = getProfilesByAccountId(changedAccount->id());
            foreach(SyncProfile *profile, profiles)
            {
                // See if the service name matches
                if(serviceName == profile->key(REMOTE_SERVICE_NAME))
                {
                    // Check if the status really changed here
                    // saving the account can trigger the emition of enabledChanged()
                    if (profile->isEnabled() != enabled) {
                        profile->setEnabled(enabled);
                        iProfileManager.updateProfile(*profile);
                        if (enabled) {
                            emit scheduleUpdated(profile->name());
                        } else {
                            emit removeScheduledSync(profile->name());
                        }
                    }
                }
                delete profile;
            }
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

        syncSchedule.setRushEnabled(account->valueAsBool(Buteo::SYNC_SCHEDULE_PEAK_ENABLED_KEY_BOOL));
        LOG_DEBUG ("Sync PEAK :" << account->valueAsBool(Buteo::SYNC_SCHEDULE_PEAK_ENABLED_KEY_BOOL));

        syncSchedule.setScheduleEnabled(account->valueAsBool(Buteo::SYNC_SCHEDULE_OFFPEAK_ENABLED_KEY_BOOL));
        LOG_DEBUG ("Sync OFFPEAK :" << account->valueAsBool(Buteo::SYNC_SCHEDULE_OFFPEAK_ENABLED_KEY_BOOL));

        int scheduleInterval = 60;
        if(syncSchedule.scheduleEnabled())
            scheduleInterval = account->valueAsInt(Buteo::SYNC_SCHEDULE_OFFPEAK_SCHEDULE_KEY_INT);
        syncSchedule.setInterval(scheduleInterval);
        LOG_DEBUG ("Sync Interval :" << scheduleInterval);

        scheduleInterval = 60;
        if(syncSchedule.rushEnabled())
            scheduleInterval = account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_SCHEDULE_KEY_INT);
        syncSchedule.setRushInterval(scheduleInterval);
        LOG_DEBUG ("Sync Rush Interval :" << scheduleInterval);

        int map = account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_DAYS_KEY_INT);
        LOG_DEBUG ("Sync Days :" << account->valueAsInt (Buteo::SYNC_SCHEDULE_PEAK_DAYS_KEY_INT));
        Buteo::DaySet rdays;
        Buteo::DaySet days;
        int lastDay = Qt::Sunday;
        while (lastDay > 0) {
            int val = 0;
            val |= (1 << (lastDay - 1));
            days.insert(lastDay);
            if ((val & map)) {
                rdays.insert(lastDay);
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
#if 0 // Not required for scheduler settings; they will be handled by slotSchedulerSettingsChanged
    FUNCTION_CALL_TRACE;
    QList<SyncProfile*> syncProfiles = getProfilesByAccountId(id);
    foreach(SyncProfile *syncProfile, syncProfiles)
    {
        if (syncProfile) {
            setSyncSchedule(syncProfile, id);
            iProfileManager.updateProfile(*syncProfile);
            emit scheduleUpdated(syncProfile->name());
            delete syncProfile;
        }
    }
#else
    Q_UNUSED(id);
#endif
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

void AccountsHelper::createProfileForAccount(Accounts::Account *account,
                                             const QString profileName,
                                             const SyncProfile *baseProfile)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Creating new profile by cloning base profile");

    // Create a new sync profile with username
    SyncProfile *newProfile = baseProfile->clone();
    newProfile->setName(profileName + "-" + QString::number(account->id()));
    newProfile->setKey(KEY_DISPLAY_NAME, profileName + "-" + account->displayName());
    newProfile->setKey(KEY_ACCOUNT_ID, QString::number(account->id()));
    // Check if account is enabled
    if (account->enabled())
        newProfile->setEnabled(true);   // Set profile as enabled

    setSyncSchedule (newProfile, account->id(), true);

    // Save the newly created profile
    iProfileManager.updateProfile(*newProfile);

    account->setValue(KEY_PROFILE_ID, newProfile->name());
    account->syncAndBlock();

    emit scheduleUpdated(newProfile->name());
    if(newProfile->isSOCProfile())
    {
        emit enableSOC(newProfile->name());
    }
    delete newProfile;
}

void AccountsHelper::addAccountIfNotExists(Accounts::Account *account,
                                           Accounts::Service service,
                                           const SyncProfile *baseProfile)
{
    FUNCTION_CALL_TRACE;

    Profile *serviceProfile = iProfileManager.profile(service.name(), Profile::TYPE_SYNC);
    if (!serviceProfile) {
        LOG_DEBUG ("!!!! Service not supported !!!!");
        return;
    }

    QString profileName ;
    if (!service.name().isEmpty()) {
        QStringList keys;
        keys << QString::number(account->id()) << service.name();
        serviceProfile->setName(keys);
        profileName = serviceProfile->name();
    }
    delete serviceProfile;

    SyncProfile *profile = iProfileManager.syncProfile(profileName);

    LOG_DEBUG("profileName:"<<profileName);

    if(0 == profile)
    {
        LOG_DEBUG("New profile creating with clone of base profile");

        // Create a new sync profile with username
        SyncProfile *newProfile = baseProfile->clone();
        newProfile->setName(profileName);
        newProfile->setKey(KEY_DISPLAY_NAME, service.name() + "-" + account->displayName());
        // Add the account ID to the profile
        newProfile->setKey(KEY_ACCOUNT_ID, QString::number(account->id()));
        // Check if service is enabled
        LOG_DEBUG("Service:: " << service.displayName() << "enabled status::" << account->enabled());
        // Set profile as enabled
        newProfile->setEnabled(account->enabled());
        setSyncSchedule (newProfile, account->id(), true);

        // Save the newly created profile
        iProfileManager.updateProfile(*newProfile);

        emit scheduleUpdated(newProfile->name());
        if(newProfile->isSOCProfile())
        {
            emit enableSOC(newProfile->name());
        }
        delete newProfile;
    }
    else if(true == profile->boolKey(KEY_USE_ACCOUNTS, false))
    {
        LOG_DEBUG("Profile already exist enable it");
        // Set profile as enabled
        profile->setEnabled(true);
        iProfileManager.updateProfile(*profile);
        emit scheduleUpdated(profile->name());
        if(profile->isSOCProfile())
        {
            emit enableSOC(profile->name());
        }
    }
    if(0 != profile)
    {
        delete profile;
    }
}

void AccountsHelper::addSetting(Accounts::AccountId id, QString key, QVariant value) {
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Account Id " << id << "   Key = " << key << "  Value = " << value);
    Accounts::Account* account = iAccountManager->account(id);
    if (account != NULL) {
        account->setValue(key, value);
        bool success = account->syncAndBlock();
        if (!success) {
            LOG_WARNING("Could not save settings to Account : Reason = " << iAccountManager->lastError().message());
        }
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

void AccountsHelper::slotSchedulerSettingsChanged(const char *aKey)
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Key Changed" << QString(aKey));
    Accounts::Watch *watch = qobject_cast<Accounts::Watch*>(this->sender());
    if(watch && iAcctWatchMap.contains(watch)) {
        Accounts::AccountId id = iAcctWatchMap.value(watch);
        QList<SyncProfile*> syncProfiles = getProfilesByAccountId(id);
        foreach(SyncProfile *syncProfile, syncProfiles)
        {
            if (syncProfile) {
                setSyncSchedule(syncProfile, id);
                iProfileManager.updateProfile(*syncProfile);
                emit scheduleUpdated(syncProfile->name());
                delete syncProfile;
            }
        }
    }
}

void AccountsHelper::registerAccountListener(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    Accounts::Account *account = iAccountManager->account(id);
    iAccountList.append(account);
    // Callback for account enabled/disabled
    QObject::connect(account, SIGNAL(enabledChanged(const QString&, bool)),
                     this, SLOT(slotAccountEnabledChanged(const QString&, bool)));

    // Account SyncOnChange
    QList<SyncProfile*> profiles = getProfilesByAccountId(id);
    foreach(SyncProfile *profile, profiles)
    {
        if(profile->isSOCProfile())
        {
            emit enableSOC(profile->name());
        }
    }
    account->selectService();
    account->beginGroup("scheduler");
    LOG_DEBUG("Watching Group :" << account->group());
    Accounts::Watch *watch = account->watchKey();
    if(!watch){
        LOG_DEBUG("Failed to add watch for acct with id:" << id);
        return;
    }
    iAcctWatchMap[watch] = id;
    QObject::connect(watch, SIGNAL(notify(const char *)), this, SLOT(slotSchedulerSettingsChanged(const char *)));
}
