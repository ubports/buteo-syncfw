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
    // Connect to signal for account creation and deletion.
    QObject::connect(iAccountManager, SIGNAL(accountCreated(Accounts::AccountId)),
                     this, SLOT(createProfileForAccount(Accounts::AccountId)));
    QObject::connect(iAccountManager, SIGNAL(accountRemoved(Accounts::AccountId)),
                     this, SLOT(slotAccountRemoved(Accounts::AccountId)));

    // load accounts after return from contructor, to allow connection with class signals
    // that can be fired by 'registerAccountListeners' function
    // Handle account modifications with a listener per account.
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

void AccountsHelper::createProfileForAccount(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    Accounts::Account *newAccount = iAccountManager->account(id);

    if (0 != newAccount) {
        registerAccountListener(id);
        bool profileFoundAndCreated = false;
        const Accounts::ServiceList serviceList = newAccount->services();
        for (const Accounts::Service &service : serviceList) {
            // Look for a sync profile that matches the service name (template)
            LOG_DEBUG("Looking for sync profile::" << service.name());
            bool serviceEnabled = newAccount->enabledServices().contains(service);
            profileFoundAndCreated =
                addProfileForAccount(newAccount, service.name(), serviceEnabled)
                || profileFoundAndCreated;
        }

        // Fetch the key "remote_service_name" from the account settings and
        // use it to create a profile
        QString serviceName = newAccount->valueAsString(REMOTE_SERVICE_NAME);
        if (profileFoundAndCreated == false && !serviceName.isEmpty()) {
            LOG_DEBUG("Profile name from account setting:" << serviceName);
            const QString profileName = serviceName + "-" + QString::number(newAccount->id());
            if (addProfileForAccount(newAccount, serviceName, true, profileName)) {
                newAccount->setValue(KEY_PROFILE_ID, profileName);
                newAccount->syncAndBlock();
            }
        }
    } else {
        LOG_DEBUG("Account not found:" << id);
    }
}

void AccountsHelper::slotAccountRemoved(Accounts::AccountId id)
{
    FUNCTION_CALL_TRACE;
    // Delete the profile(s) with account ID
    QList<SyncProfile *> syncProfiles = getProfilesByAccountId(id);
    foreach (SyncProfile *profile, syncProfiles) {
        LOG_DEBUG("Removing profile" << profile->name());
        emit removeProfile(profile->name());
        delete profile;
    }
#ifdef USE_ACCOUNTSHELPER_SCHEDULER_WATCHER
    // remove corresponding watch from the Map
    QMap<Accounts::Watch *, Accounts::AccountId>::iterator i = iAcctWatchMap.begin();
    while (i != iAcctWatchMap.end()) {
        if (i.value() == id) {
            i = iAcctWatchMap.erase(i);
            break;
        } else
            ++i;
    }
#endif
}

static Accounts::Service serviceForProfile(Accounts::Account *account,
                                           const SyncProfile *profile)
{
    const Accounts::ServiceList services = account->services();
    for (const Accounts::Service &service : services) {
        account->selectService(service);
        // Look for profile naming using a template defined in the account service.
        const QStringList names = account->value(QStringLiteral("sync_profile_templates")).toStringList();
        for (const QString &name : names) {
            if (profile->name() == name + "-" + QString::number(account->id())) {
                return service;
            }
        }
        // Earlier possible matching was using service name as profile name.
        // This was abandonned later to avoid profile name clashing with
        // identical services for different accounts.
        if (profile->name() == service.name()) {
            return service;
        }
        // Add here other possible matcher schemes.
    }
    return Accounts::Service();
}

void AccountsHelper::syncEnableWithAccount(Accounts::Account *account)
{
    account->selectService();
    // Always use the current enabled value since signals may be emitted with delays.
    bool enabled = account->enabled();
    const QList<SyncProfile *> profiles = getProfilesByAccountId(account->id());
    for (SyncProfile *profile : profiles) {
        LOG_DEBUG("Changing profile enabled" << profile->name() << enabled);
        if (enabled) {
            // Global is enabled, checking by service if any.
            bool serviceEnabled = true;
            Accounts::Service service = serviceForProfile(account, profile);
            if (service.isValid()) {
                account->selectService(service);
                serviceEnabled = account->enabled();
            }
            LOG_DEBUG("Enabled status for service ::" << profile->name() << serviceEnabled);
            if (profile->isEnabled() != serviceEnabled) {
                profile->setEnabled(serviceEnabled);
                iProfileManager.updateProfile(*profile);
                emit scheduleUpdated(profile->name());
            }
        } else if (profile->isEnabled()) {
            // Global is false, unconditionally disable
            profile->setEnabled(false);
            iProfileManager.updateProfile(*profile);
            emit removeScheduledSync(profile->name());
        }
        delete profile;
    }
    account->selectService();
}

void AccountsHelper::setSyncSchedule(SyncProfile *syncProfile, Accounts::AccountId id, bool aCreateNew)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (aCreateNew);
    Accounts::Account *account = iAccountManager->account(id);
    if (0 != account) {
        //Sync schedule settings should be global
        account->selectService();
        account->beginGroup("scheduler");
        SyncSchedule syncSchedule;

        int peakStart = account->valueAsInt(Buteo::SYNC_SCHEDULE_PEAK_START_TIME_KEY_INT);
        QTime startTime(peakStart / 60, peakStart % 60);

        int peakEnd = account->valueAsInt(Buteo::SYNC_SCHEDULE_PEAK_END_TIME_KEY_INT);
        QTime endTime(peakEnd / 60, peakEnd % 60);
        LOG_DEBUG ("Start time:" << startTime << "End Time :" << endTime);
        syncSchedule.setRushTime(startTime, endTime);

        SyncProfile::SyncType syncType = account->valueAsBool (Buteo::SYNC_SCHEDULE_ENABLED_KEY_BOOL) ?
                                         SyncProfile::SYNC_SCHEDULED : SyncProfile::SYNC_MANUAL ;
        syncProfile->setSyncType (syncType);
        LOG_DEBUG ("Sync Type :" << syncType );

        syncSchedule.setRushEnabled(account->valueAsBool(Buteo::SYNC_SCHEDULE_PEAK_ENABLED_KEY_BOOL));
        LOG_DEBUG ("Sync PEAK :" << account->valueAsBool(Buteo::SYNC_SCHEDULE_PEAK_ENABLED_KEY_BOOL));

        syncSchedule.setScheduleEnabled(account->valueAsBool(Buteo::SYNC_SCHEDULE_OFFPEAK_ENABLED_KEY_BOOL));
        LOG_DEBUG ("Sync OFFPEAK :" << account->valueAsBool(Buteo::SYNC_SCHEDULE_OFFPEAK_ENABLED_KEY_BOOL));

        int scheduleInterval = 60;
        if (syncSchedule.scheduleEnabled())
            scheduleInterval = account->valueAsInt(Buteo::SYNC_SCHEDULE_OFFPEAK_SCHEDULE_KEY_INT);
        syncSchedule.setInterval(scheduleInterval);
        LOG_DEBUG ("Sync Interval :" << scheduleInterval);

        scheduleInterval = 60;
        if (syncSchedule.rushEnabled())
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
        account->endGroup();
        syncProfile->setSyncSchedule (syncSchedule);
    }
}

QList<SyncProfile *> AccountsHelper::getProfilesByAccountId(Accounts::AccountId id)
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

bool AccountsHelper::addProfileForAccount(Accounts::Account *account,
                                          const QString &serviceName,
                                          bool serviceEnabled,
                                          const QString &label)
{
    FUNCTION_CALL_TRACE;

    SyncProfile *serviceProfile = iProfileManager.syncProfile(serviceName);
    if (!serviceProfile || (false == serviceProfile->boolKey(KEY_USE_ACCOUNTS, false))) {
        LOG_DEBUG ("!!!! Service not supported !!!!");
        return false;
    }

    QString profileName = label;
    if (profileName.isEmpty()) {
        QStringList keys;
        keys << QString::number(account->id()) << serviceName;
        serviceProfile->setName(keys);
        profileName = serviceProfile->name();
    }

    SyncProfile *profile = iProfileManager.syncProfile(profileName);

    LOG_DEBUG("profileName:" << profileName);

    if (0 == profile) {
        LOG_DEBUG("Creating new profile by cloning base profile");

        // Create a new sync profile with username
        profile = serviceProfile->clone();
        profile->setName(profileName);
        profile->setKey(KEY_DISPLAY_NAME, account->displayName());
        profile->setKey(KEY_ACCOUNT_ID, QString::number(account->id()));

        setSyncSchedule(profile, account->id(), true);
    }
    if (profile && (true == profile->boolKey(KEY_USE_ACCOUNTS, false))) {
        profile->setEnabled(account->enabled() && serviceEnabled);
        iProfileManager.updateProfile(*profile);
        emit scheduleUpdated(profile->name());
        if (profile->isSOCProfile()) {
            emit enableSOC(profile->name());
        }
    }
    delete profile;
    delete serviceProfile;

    return true;
}

void AccountsHelper::registerAccountListeners()
{
    FUNCTION_CALL_TRACE;
    // Populate all enabled accounts list (so that we can listen to changes)
    QList<Accounts::AccountId> accountIds = iAccountManager->accountList();
    foreach (Accounts::AccountId id, accountIds) {
        registerAccountListener(id);
    }
}

void AccountsHelper::slotSchedulerSettingsChanged(const char *aKey)
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Key Changed" << QString(aKey));
    Accounts::Watch *watch = qobject_cast<Accounts::Watch *>(this->sender());
    if (watch && iAcctWatchMap.contains(watch)) {
        Accounts::AccountId id = iAcctWatchMap.value(watch);
        QList<SyncProfile *> syncProfiles = getProfilesByAccountId(id);
        foreach (SyncProfile *syncProfile, syncProfiles) {
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
    if (iAccountList.contains(account)) {
        return;
    }
    iAccountList.append(account);
    // Initialisation and callback for account enabled/disabled
    syncEnableWithAccount(account);
    QObject::connect(account, &Accounts::Account::enabledChanged,
    [this, account] (const QString & serviceName, bool enabled) {
        LOG_DEBUG("Received account enabled changed signal" << serviceName << enabled << account->displayName());
        syncEnableWithAccount(account);
    });

#ifndef USE_ACCOUNTSHELPER_SCHEDULER_WATCHER
    LOG_DEBUG("AccountsHelper::registerAccountListener() is disabled!  Not listening to scheduler change signals for account:"
              << id);
#else
    // Account SyncOnChange
    QList<SyncProfile *> profiles = getProfilesByAccountId(id);
    foreach (SyncProfile *profile, profiles) {
        if (profile->isSOCProfile()) {
            emit enableSOC(profile->name());
        }
    }
    account->selectService();
    account->beginGroup("scheduler");
    LOG_DEBUG("Watching Group :" << account->group());
    Accounts::Watch *watch = account->watchKey();
    account->endGroup();
    if (!watch) {
        LOG_DEBUG("Failed to add watch for acct with id:" << id);
        return;
    }
    iAcctWatchMap[watch] = id;
    QObject::connect(watch,
                     SIGNAL(notify(const char *)),
                     SLOT(slotSchedulerSettingsChanged(const char *)),
                     Qt::UniqueConnection);
#endif
}
