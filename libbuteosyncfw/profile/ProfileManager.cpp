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

#include "ProfileManager.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>

#include "ProfileFactory.h"
#include "ProfileEngineDefs.h"
#include "SyncCommonDefs.h"

#include "LogMacros.h"
#include "BtHelper.h"

namespace Buteo {

static const QString FORMAT_EXT = ".xml";
static const QString BACKUP_EXT = ".bak";
static const QString LOG_EXT = ".log";
static const QString LOG_DIRECTORY = "logs";
static const QString BT_PROFILE_TEMPLATE("bt_template");

const QString ProfileManager::DEFAULT_PRIMARY_PROFILE_PATH =
        Sync::syncCacheDir();
const QString ProfileManager::DEFAULT_SECONDARY_PROFILE_PATH =
        "/etc/buteo/profiles";

// Private implementation class for ProfileManager.
class ProfileManagerPrivate
{
public:
    ProfileManagerPrivate(const QString &aPrimaryPath,
            const QString &aSecondaryPath);

    /*! \brief Loads a profile from persistent storage.
     *
     * \param aName Name of the profile to load.
     * \param aType Type of the profile to load.
     * \return The loaded profile. 0 if the profile was not found.
     */
    Profile *load(const QString &aName, const QString &aType);

    /*! \brief Loads the synchronization log associated with the given profile.
     *
     * \param aProfileName Name of the sync profile whose log shall be loaded.
     * \return The loaded log. 0 if the log was not found.
     */
    SyncLog *loadLog(const QString &aProfileName);

    bool parseFile(const QString &aPath, QDomDocument &aDoc);

    void restoreBackupIfFound(const QString &aProfilePath,
            const QString &aBackupPath);

    QDomDocument constructProfileDocument(const Profile &aProfile);

    bool writeProfileFile(const QString &aProfilePath, const QDomDocument &aDoc);

    QString findProfileFile(const QString &aName, const QString &aType);

    bool createBackup(const QString &aProfilePath, const QString &aBackupPath);

    bool matchProfile(const Profile &aProfile,
            const ProfileManager::SearchCriteria &aCriteria);

    bool matchKey(const Profile &aProfile,
            const ProfileManager::SearchCriteria &aCriteria);

    bool save(const Profile &aProfile);

    bool remove(const QString &aName, const QString &aType);

    bool profileExists(const QString &aProfileId ,const QString &aType);

    // Primary path for profiles.
    QString iPrimaryPath;

    // Secondary path for profiles.
    QString iSecondaryPath;


};

}

using namespace Buteo;

ProfileManagerPrivate::ProfileManagerPrivate(const QString &aPrimaryPath,
        const QString &aSecondaryPath)
:   iPrimaryPath(aPrimaryPath),
    iSecondaryPath(aSecondaryPath)
{

    if (iPrimaryPath.endsWith(QDir::separator()))
    {
        iPrimaryPath.chop(1);
    } // no else
    if (iSecondaryPath.endsWith(QDir::separator()))
    {
        iSecondaryPath.chop(1);
    } // no else

    LOG_DEBUG("Primary profile path set to" << iPrimaryPath);
    LOG_DEBUG("Secondary profile path set to" << iSecondaryPath);
}

Profile *ProfileManagerPrivate::load(const QString &aName, const QString &aType)
{

    QString profilePath = findProfileFile(aName, aType);
    QString backupProfilePath = profilePath + BACKUP_EXT;

    QDomDocument doc;
    Profile* profile = 0;

    restoreBackupIfFound(profilePath, backupProfilePath);

    if (parseFile(profilePath, doc))
    {
        ProfileFactory pf;
        profile = pf.createProfile(doc.documentElement());

        if (QFile::exists(backupProfilePath))
        {
            QFile::remove(backupProfilePath);
        }
    }
    else {
        LOG_WARNING("Failed to load profile:" << aName);
    }

    return profile;
}

SyncLog *ProfileManagerPrivate::loadLog(const QString &aProfileName)
{


    QString fileName = iPrimaryPath + QDir::separator() + Profile::TYPE_SYNC + QDir::separator() +
            LOG_DIRECTORY + QDir::separator() + aProfileName + LOG_EXT + FORMAT_EXT;

    if (!QFile::exists(fileName))
    {
        LOG_DEBUG("No sync log found for profile:" << aProfileName);
        return 0;
    } // no else

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_WARNING("Failed to open sync log file for reading:"
                << file.fileName());
        return 0;
    } // no else

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        LOG_WARNING("Failed to parse XML from sync log file:"
                << file.fileName());
        return 0;
    } // no else
    file.close();

    return new SyncLog(doc.documentElement());
}

bool ProfileManagerPrivate::matchProfile(const Profile &aProfile,
        const ProfileManager::SearchCriteria &aCriteria)
{
    //FUNCTION_CALL_TRACE;
    bool matched = false;

    const Profile *testProfile = &aProfile;
    if (!aCriteria.iSubProfileName.isEmpty())
    {
        // Sub-profile name was given, request a sub-profile with a
        // matching name and type.
        testProfile = aProfile.subProfile(aCriteria.iSubProfileName,
                aCriteria.iSubProfileType);

        if (testProfile != 0)
        {
            matched = matchKey(*testProfile, aCriteria);
        }
        else
        {
            if (aCriteria.iType == ProfileManager::SearchCriteria::NOT_EXISTS)
            {
                matched = true;
            }
            else
            {
                matched = false;
            }
        }
    }
    else if (!aCriteria.iSubProfileType.isEmpty())
    {
        // Sub-profile name was empty, but type was given. Get all
        // sub-profiles with the matching type.
        QStringList subProfileNames =
                aProfile.subProfileNames(aCriteria.iSubProfileType);
        if (!subProfileNames.isEmpty())
        {
            matched = false;
            foreach (const QString &subProfileName, subProfileNames)
            {
                testProfile = aProfile.subProfile(subProfileName,
                        aCriteria.iSubProfileType);
                if (testProfile != 0 && matchKey(*testProfile, aCriteria))
                {
                    matched = true;
                    break;
                }
            }
        }
        else
        {
            if (aCriteria.iType == ProfileManager::SearchCriteria::NOT_EXISTS)
            {
                matched = true;
            }
            else
            {
                matched = false;
            }
        }
    }
    else
    {
        matched = matchKey(aProfile, aCriteria);
    }

    return matched;
}

bool ProfileManagerPrivate::matchKey(const Profile &aProfile,
        const ProfileManager::SearchCriteria &aCriteria)
{
    bool matched = false;

    if (!aCriteria.iKey.isEmpty())
    {
        // Key name was given, get a key with matching name.
        QString value = aProfile.key(aCriteria.iKey);

        if (value.isNull())
        {
            if (aCriteria.iType == ProfileManager::SearchCriteria::NOT_EXISTS ||
                    aCriteria.iType == ProfileManager::SearchCriteria::NOT_EQUAL)
            {
                matched = true;
            }
            else
            {
                matched = false;
            }
        }
        else
        {
            switch (aCriteria.iType)
            {
            case ProfileManager::SearchCriteria::EXISTS:
                matched = true;
                break;

            case ProfileManager::SearchCriteria::NOT_EXISTS:
                matched = false;
                break;

            case ProfileManager::SearchCriteria::EQUAL:
                matched = (value == aCriteria.iValue);
                break;

            case ProfileManager::SearchCriteria::NOT_EQUAL:
                matched = (value != aCriteria.iValue);
                break;

            default:
                matched = false;
                break;
            }
        }
    }
    else
    {
        if (aCriteria.iType == ProfileManager::SearchCriteria::NOT_EXISTS)
        {
            matched = false;
        }
        else
        {
            matched = true;
        }
    }

    return matched;
}

ProfileManager::SearchCriteria::SearchCriteria()
:  iType(ProfileManager::SearchCriteria::EQUAL)
{
}

ProfileManager::SearchCriteria::SearchCriteria(const SearchCriteria &aSource)
:  iType(aSource.iType),
   iSubProfileName(aSource.iSubProfileName),
   iSubProfileType(aSource.iSubProfileType),
   iKey(aSource.iKey),
   iValue(aSource.iValue)
{
}

ProfileManager::ProfileManager(const QString &aPrimaryPath,
        const QString &aSecondaryPath)
:   d_ptr(new ProfileManagerPrivate(aPrimaryPath, aSecondaryPath))
{
    FUNCTION_CALL_TRACE;
}

ProfileManager::~ProfileManager()
{
    FUNCTION_CALL_TRACE;
    delete d_ptr;
    d_ptr = 0;
}

Profile *ProfileManager::profile(const QString &aName, const QString &aType)
{

    return d_ptr->load(aName, aType);
}

SyncProfile *ProfileManager::syncProfile(const QString &aName)
{

    Profile *p = profile(aName, Profile::TYPE_SYNC);
    SyncProfile *syncProfile = 0;
    if (p != 0 && p->type() == Profile::TYPE_SYNC)
    {
        // RTTI is not allowed, use static_cast. Should be safe, because
        // type is verified.
        syncProfile = static_cast<SyncProfile*>(p);

        // Load and merge all sub-profiles.
        expand(*syncProfile);

        // Load sync log. If not found, create an empty log.
        if (syncProfile->log() == 0)
        {
            SyncLog *log = d_ptr->loadLog(aName);
            if (0 == log)
            {
                log = new SyncLog(aName);
            } // no else
            syncProfile->setLog(log);
        } // no else
    } else {
        if (p != 0) {
            delete p;
        }
    }

    return syncProfile;
}

QStringList ProfileManager::profileNames(const QString &aType)
{

    // Search for all profile files from the primary directory
    QStringList names;
    QString nameFilter = QString("*") + FORMAT_EXT;
    {
        QDir dir(d_ptr->iPrimaryPath + QDir::separator() + aType);
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList(nameFilter),
                QDir::Files | QDir::NoSymLinks);
        foreach (const QFileInfo &fileInfo, fileInfoList)
        {
            names.append(fileInfo.completeBaseName());
        }
    }

    // Search for all profile files from the secondary directory
    {
        QDir dir(d_ptr->iSecondaryPath + QDir::separator() + aType);
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList(nameFilter),
                QDir::Files | QDir::NoSymLinks);
        foreach (const QFileInfo &fileInfo, fileInfoList)
        {
            // Add only if the list does not yet contain the name.
            QString profileName = fileInfo.completeBaseName();
            if (!names.contains(profileName))
            {
                names.append(profileName);
            }
        }
    }

    return names;
}

QList<SyncProfile*> ProfileManager::allSyncProfiles()
{
    FUNCTION_CALL_TRACE;

    QList<SyncProfile*> profiles;

    QStringList names = profileNames(Profile::TYPE_SYNC);
    foreach (const QString &name, names)
    {
        SyncProfile *p = syncProfile(name);
        if (p != 0)
        {
            profiles.append(p);
        } // no else
    }

    return profiles;
}

QList<SyncProfile*> ProfileManager::allVisibleSyncProfiles()
{
    FUNCTION_CALL_TRACE;

    QList<SyncProfile*> profiles = allSyncProfiles();
    QList<SyncProfile*> visibleProfiles;
    foreach (SyncProfile *p, profiles)
    {
        if (!p->isHidden())
        {
            visibleProfiles.append(p);
        }
        else
        {
            delete p;
        }
    }

    return visibleProfiles;
}

QList<SyncProfile*> ProfileManager::getSyncProfilesByData(
        const QString &aSubProfileName,
        const QString &aSubProfileType,
        const QString &aKey, const QString &aValue)
{
    FUNCTION_CALL_TRACE;

    QList<SyncProfile*> allProfiles = allSyncProfiles();
    QList<SyncProfile*> matchingProfiles;

    foreach (SyncProfile *profile, allProfiles)
    {
        Profile *testProfile = profile;
        if (!aSubProfileName.isEmpty())
        {
            // Sub-profile name was given, request a sub-profile with a
            // matching name and type.
            testProfile = profile->subProfile(aSubProfileName, aSubProfileType);
        }
        else if (!aSubProfileType.isEmpty())
        {
            // Sub-profile name was empty, but type was given. Get the first
            // sub-profile with the matching type.
            QStringList subProfileNames =
                    profile->subProfileNames(aSubProfileType);
            if (!subProfileNames.isEmpty())
            {
                testProfile = profile->subProfile(subProfileNames.first(),
                        aSubProfileType);
            }
            else
            {
                testProfile = 0;
            }
        }

        if (0 == testProfile) // Sub-profile was not found.
        {
            delete profile;
            profile = 0;
            continue; // Not a match, continue with next profile.
        }

        if (!aKey.isEmpty())
        {
            // Key name was given, get a key with matching name.
            QString value = testProfile->key(aKey);
            if (value.isNull() || // Key was not found.
                    (!aValue.isEmpty() && (value != aValue))) // Value didn't match
            {
                delete profile;
                profile = 0;
                continue; // Not a match, continue with next profile.
            } // no else
        } // no else

        // Match, add profile to the list to be returned.
        matchingProfiles.append(profile);
    }

    return matchingProfiles;
}

QList<SyncProfile*> ProfileManager::getSyncProfilesByData(
        const QList<SearchCriteria> &aCriteria)
{
    FUNCTION_CALL_TRACE;

    QList<SyncProfile*> allProfiles = allSyncProfiles();
    QList<SyncProfile*> matchingProfiles;

    foreach (SyncProfile *profile, allProfiles)
    {
        bool matched = true;
        if (profile == 0)
            continue;

        foreach (const SearchCriteria &criteria, aCriteria)
        {
            if (!d_ptr->matchProfile(*profile, criteria))
            {
                matched = false;
                break;
            }
        }

        if (matched)
        {
            matchingProfiles.append(profile);
        }
        else
        {
            delete profile;
            profile = 0;
        }
    }

    return matchingProfiles;
}

QList<SyncProfile*> ProfileManager::getSOCProfilesForStorage(
        const QString &aStorageName)
{
    FUNCTION_CALL_TRACE;

    QList<SearchCriteria> criteriaList;

    // Require that the profile is not disabled.
    // Profile is enabled by default. Comparing with enabled = true would
    // not work, because the key may not exist at all, even if the profile
    // is enabled.
    SearchCriteria profileEnabled;
    profileEnabled.iType = SearchCriteria::NOT_EQUAL;
    profileEnabled.iKey = KEY_ENABLED;
    profileEnabled.iValue = BOOLEAN_FALSE;
    criteriaList.append(profileEnabled);

    // Profile must not be hidden.
    SearchCriteria profileVisible;
    profileVisible.iType = SearchCriteria::NOT_EQUAL;
    profileVisible.iKey = KEY_HIDDEN;
    profileVisible.iValue = BOOLEAN_TRUE;
    criteriaList.append(profileVisible);

    // Online service.
    SearchCriteria onlineService;
    onlineService.iType = SearchCriteria::EQUAL;
    //onlineService.iSubProfileType = Profile::TYPE_SERVICE;
    // Service profile name is left empty. Key value is matched with all
    // found service sub-profiles, though there should be only one.
    onlineService.iKey = KEY_DESTINATION_TYPE;
    onlineService.iValue = VALUE_ONLINE;
    criteriaList.append(onlineService);

    // The profile should be interested
    // in SOC
    SearchCriteria socSupported;
    //socSupported.iSubProfileType = Profile::TYPE_SERVICE;
    socSupported.iType = SearchCriteria::EQUAL;
    socSupported.iKey = KEY_SOC;
    socSupported.iValue = BOOLEAN_TRUE;
    criteriaList.append(socSupported);

    SearchCriteria storageSupported;
    storageSupported.iSubProfileType = Profile::TYPE_STORAGE;
    storageSupported.iType = SearchCriteria::EQUAL;
    storageSupported.iKey = KEY_LOCAL_URI;
    storageSupported.iValue = aStorageName;
    criteriaList.append(storageSupported);

    return getSyncProfilesByData(criteriaList);
}

QList<SyncProfile*> ProfileManager::getSyncProfilesByStorage(
        const QString &aStorageName, bool aStorageMustBeEnabled)
{
    FUNCTION_CALL_TRACE;

    QList<SearchCriteria> criteriaList;

    // Require that the profile is not disabled.
    // Profile is enabled by default. Comparing with enabled = true would
    // not work, because the key may not exist at all, even if the profile
    // is enabled.
    SearchCriteria profileEnabled;
    profileEnabled.iType = SearchCriteria::NOT_EQUAL;
    profileEnabled.iKey = KEY_ENABLED;
    profileEnabled.iValue = BOOLEAN_FALSE;
    criteriaList.append(profileEnabled);

    // Profile must not be hidden.
    SearchCriteria profileVisible;
    profileVisible.iType = SearchCriteria::NOT_EQUAL;
    profileVisible.iKey = KEY_HIDDEN;
    profileVisible.iValue = BOOLEAN_TRUE;
    criteriaList.append(profileVisible);

    // Online service.
    SearchCriteria onlineService;
    onlineService.iType = SearchCriteria::EQUAL;
    //onlineService.iSubProfileType = Profile::TYPE_SERVICE;
    // Service profile name is left empty. Key value is matched with all
    // found service sub-profiles, though there should be only one.
    onlineService.iKey = KEY_DESTINATION_TYPE;
    onlineService.iValue = VALUE_ONLINE;
    criteriaList.append(onlineService);

    // Storage must be supported.
    SearchCriteria storageSupported;
    storageSupported.iSubProfileName = aStorageName;
    storageSupported.iSubProfileType = Profile::TYPE_STORAGE;
    if (aStorageMustBeEnabled)
    {
        // Storage must be enabled also. Storages are disabled by default,
        // so we can compare with enabled = true.
        storageSupported.iType = SearchCriteria::EQUAL;
        storageSupported.iKey = KEY_ENABLED;
        storageSupported.iValue = BOOLEAN_TRUE;
    }
    else
    {
        // Existence of the storage sub-profile is sufficient.
        storageSupported.iType = SearchCriteria::EXISTS;
    }
    criteriaList.append(storageSupported);

    return getSyncProfilesByData(criteriaList);
}


bool ProfileManagerPrivate::save(const Profile &aProfile)
{
    FUNCTION_CALL_TRACE;

    QDomDocument doc = constructProfileDocument(aProfile);
    if (doc.isNull())
    {
        LOG_WARNING("No profile data to write");
        return false;
    }

    // Create path for the new profile file.
    QDir dir;
    dir.mkpath(iPrimaryPath + QDir::separator() + aProfile.type());
    QString profilePath(iPrimaryPath + QDir::separator() +
            aProfile.type() + QDir::separator() + aProfile.name() + FORMAT_EXT);

    // Create a backup of the existing profile file.
    QString oldProfilePath = findProfileFile(aProfile.type(), aProfile.name());
    QString backupPath = profilePath + BACKUP_EXT;

    if (QFile::exists(oldProfilePath) &&
            !createBackup(oldProfilePath, backupPath))
    {
        LOG_WARNING("Failed to create profile backup");
    }

    bool profileWritten = false;
    if (writeProfileFile(profilePath, doc))
    {
        QFile::remove(backupPath);
        profileWritten = true;
    }
    else
    {
        LOG_WARNING("Failed to save profile:" << aProfile.name());
        profileWritten = false;
    }

    return profileWritten;
}

Profile* ProfileManager::profileFromXml(const QString &aProfileAsXml)
{
    FUNCTION_CALL_TRACE;

    Profile *profile = NULL;
    if(!aProfileAsXml.isEmpty()) {
        QDomDocument doc;
        if(doc.setContent(aProfileAsXml,true)) {
            ProfileFactory pf;
            profile = pf.createProfile(doc.documentElement());
        }
    }
    return profile;
}

QString ProfileManager::updateProfile(const Profile &aProfile)
{
    FUNCTION_CALL_TRACE;

    // We must have a profile exiting before updating it...

    bool exists = d_ptr->profileExists(aProfile.name(),aProfile.type());

    QString profileId("");
    // Profile did not exist, it was a new one. Add it and emit signal with "added" value:
    if (!exists) {
        emit signalProfileChanged(aProfile.name(),ProfileManager::PROFILE_ADDED,aProfile.toString());
    } else {
        emit signalProfileChanged(aProfile.name(),ProfileManager::PROFILE_MODIFIED,aProfile.toString());
    }

    if(d_ptr->save(aProfile)) {
        profileId = aProfile.name();
    }
    return profileId;
}

SyncProfile *ProfileManager::createTempSyncProfile (const QString &destAddress, bool &saveNewProfile)
{
    FUNCTION_CALL_TRACE;

    if (destAddress.contains("USB")) { //USB - PCSUite no requirement to save profile
        LOG_INFO("USB connect - pc");
        SyncProfile *profile = new SyncProfile(PC_SYNC);
        profile->setBoolKey(KEY_HIDDEN, true);
        profile->setKey(KEY_DISPLAY_NAME, PC_SYNC);
        return profile;
    }

    BtHelper btHelp(destAddress);
    QMap <QString , QVariant> mapVal = btHelp.getDeviceProperties();
    uint classType = mapVal.value("Class").toInt();
    uint pcsuiteClass = 0x100; //Major Device Class - Computer!

    if (classType & pcsuiteClass) {
        LOG_INFO("Device major class is Computer"); // not required to save profile
        SyncProfile *profile = new SyncProfile(PC_SYNC);
        profile->setBoolKey(KEY_HIDDEN, true);
        profile->setKey(KEY_DISPLAY_NAME, PC_SYNC);
        return profile;
    }

    saveNewProfile = true;
    QString profileDisplayName = mapVal.value("Name").toString();
    if (profileDisplayName.isEmpty()) {
        //Todo : What to show if name is empty !!
        //Fixes 171340
        profileDisplayName = QString ("qtn_sync_dest_name_device_default");
    }

    LOG_INFO("Profile Name :" << profileDisplayName);
    SyncProfile *tProfile = syncProfile(BT_PROFILE_TEMPLATE);
    //Profile *service = tProfile->serviceProfile();
    //if (service != 0) {
        tProfile->setKey(KEY_DISPLAY_NAME, profileDisplayName);
        QStringList keys ;
        keys << destAddress << tProfile->name();
        tProfile->setName(keys);
        tProfile->setEnabled(true);
        tProfile->setBoolKey("hidden", false);
        tProfile->setKey(KEY_BT_ADDRESS, destAddress);
        tProfile->setKey(KEY_BT_NAME, profileDisplayName);
    //} else {
    //   LOG_WARNING("No service profile, unable to update properties");
    //}

    return tProfile;
}


void ProfileManager::enableStorages(Profile &aProfile,
                                    QMap<QString, bool> &aStorageMap)
{
    FUNCTION_CALL_TRACE;

    QMapIterator<QString, bool> i(aStorageMap);
    LOG_INFO("ProfileManager::enableStorages");
    while (i.hasNext()) {
        i.next();
        Profile *profile = aProfile.subProfile(i.key(), Profile::TYPE_STORAGE);
        if (profile)
            profile->setEnabled(i.value());
        else
            LOG_WARNING("No storage profile by key :" << i.key());
    }
    return ;
}

void ProfileManager::setStoragesVisible(Profile &aProfile,
                                        QMap<QString, bool> &aStorageMap)
{
    FUNCTION_CALL_TRACE;

    QMapIterator<QString, bool> i(aStorageMap);
    LOG_INFO("ProfileManager::enableStorages");
    while (i.hasNext()) {
        i.next();
        Profile *profile = aProfile.subProfile(i.key(), Profile::TYPE_STORAGE);

        if (profile) {
            // For setting the "hidden" value to correspond visiblity, invert the value from map.
            profile->setBoolKey(Buteo::KEY_HIDDEN, !i.value());
        } else {
            LOG_WARNING("No storage profile by key :" << i.key());
        }
    }
    return ;
}

bool ProfileManager::removeProfile(const QString &aProfileId)
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    SyncProfile *profile = syncProfile(aProfileId);
    if(profile){
       success = d_ptr->remove(aProfileId,profile->type());
       if(success) {
           emit signalProfileChanged(aProfileId,ProfileManager::PROFILE_REMOVED, QString(""));
       }
       delete profile;
       profile = NULL;
    }
    return success;
}

bool ProfileManagerPrivate::remove(const QString &aName, const QString &aType)
{
    FUNCTION_CALL_TRACE;

    bool success = false;
    QString filePath = iPrimaryPath + QDir::separator() + aType + QDir::separator() + aName + FORMAT_EXT;

    // Try to load profile without expanding it. We need to check from the
    // profile data if the profile is protected before removing it.
    Profile *p = load(aName, aType);
    if (p)
    {
        if (!p->isProtected())
        {
            success = QFile::remove(filePath);
            if (success){
               QString logFilePath = iPrimaryPath + QDir::separator() + aType + QDir::separator() +
                        LOG_DIRECTORY + QDir::separator() + aName + LOG_EXT + FORMAT_EXT;
               //Initial the will be no log this will fail.
               QFile::remove(logFilePath);
            }
        }
        else
        {
            LOG_DEBUG( "Cannot remove protected profile:" << aName );
        }
        delete p;
        p = 0;
    }
    else
    {
        LOG_DEBUG( "Profile not found from the primary path, cannot remove:" << aName );
    }

    return success;
}

void ProfileManager::expand(Profile &aProfile)
{

    if (aProfile.isLoaded())
        return; // Already expanded.

    // Load and merge sub-profiles.
    int prevSubCount = 0;
    QList<Profile*> subProfiles = aProfile.allSubProfiles();
    int subCount = subProfiles.size();
    while (subCount > prevSubCount)
    {
        foreach (Profile *sub, subProfiles)
                        {
            if (!sub->isLoaded())
            {
                Profile *loadedProfile = profile(sub->name(), sub->type());
                if (loadedProfile != 0)
                {
                    aProfile.merge(*loadedProfile);
                    delete loadedProfile;
                    loadedProfile = 0;
                }
                else
                {
                    // No separate profile file for the sub-profile.
                    LOG_DEBUG( "Referenced sub-profile not found:" <<
                            sub->name() );
                    LOG_DEBUG( "Referenced from:" << aProfile.name() <<
                            aProfile.type() );
                }
                sub->setLoaded(true);
            } // no else
                        }

        // Load/merge may have created new sub-profile entries. Those need
        // to be loaded also. Loop if sub-profile count has changed.
        prevSubCount = subCount;
        subProfiles = aProfile.allSubProfiles();
        subCount = subProfiles.size();
    }

    aProfile.setLoaded(true);
}

bool ProfileManager::saveLog(const SyncLog &aLog)
{
    FUNCTION_CALL_TRACE;

    QDir dir;
    QString fullPath = d_ptr->iPrimaryPath + QDir::separator() + Profile::TYPE_SYNC + QDir::separator() +
            LOG_DIRECTORY;
    dir.mkpath(fullPath);
    QFile file(fullPath + QDir::separator() + aLog.profileName() + LOG_EXT + FORMAT_EXT);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        LOG_WARNING("Failed to open sync log file for writing:"
                << file.fileName());
        return false;
    } // no else

    QDomDocument doc;
    QDomProcessingInstruction xmlHeading =
            doc.createProcessingInstruction("xml",
                    "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlHeading);

    QDomElement root = aLog.toXml(doc);
    if (root.isNull())
    {
        LOG_WARNING("Failed to convert sync log to XML");
        return false;
    } // no else

    doc.appendChild(root);

    QTextStream outputStream(&file);

    outputStream << doc.toString(PROFILE_INDENT);

    file.close();

    return true;
}

void ProfileManager::saveRemoteTargetId(Profile &aProfile,const QString& aTargetId )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("saveRemoteTargetId :" << aTargetId);
    aProfile.setKey (KEY_REMOTE_ID, aTargetId);
    updateProfile(aProfile);
    //addProfile(aProfile);
}


bool ProfileManager::rename(const QString &aName, const QString &aNewName)
{
    FUNCTION_CALL_TRACE;

    bool ret = false;
    // Rename the sync profile
    QString source = d_ptr->iPrimaryPath + QDir::separator() +  Profile::TYPE_SYNC + QDir::separator() +
            aName + FORMAT_EXT;
    QString destination = d_ptr->iPrimaryPath + QDir::separator() + Profile::TYPE_SYNC + QDir::separator() +
            aNewName + FORMAT_EXT;
    ret = QFile::rename(source, destination);
    if(true == ret)
    {
        // Rename the sync log
        QString sourceLog = d_ptr->iPrimaryPath + QDir::separator() +  Profile::TYPE_SYNC + QDir::separator() +
                LOG_DIRECTORY + QDir::separator() + aName + LOG_EXT  + FORMAT_EXT;
        QString destinationLog = d_ptr->iPrimaryPath + QDir::separator() +  Profile::TYPE_SYNC + QDir::separator() +
                LOG_DIRECTORY + QDir::separator() + aNewName + LOG_EXT  + FORMAT_EXT;
        ret = QFile::rename(sourceLog, destinationLog);
        if(false == ret)
        {
            // Roll back the earlier rename
            QFile::rename(destination, source);
        }
    }
    if(false == ret)
    {
        LOG_WARNING("Failed to rename profile" << aName);
    }
    return ret;
}

bool ProfileManager::saveSyncResults(QString aProfileName,
        const SyncResults &aResults)
{

    FUNCTION_CALL_TRACE;
    bool success = false;

    SyncProfile *profile = syncProfile(aProfileName);
    if (profile) {
        SyncLog *log = profile->log();
        if (log)
        {
            log->addResults(aResults);
            success = saveLog(*log);
            //Emitting signal
            emit signalProfileChanged(aProfileName,ProfileManager::PROFILE_LOGS_MODIFIED,profile->toString());
        }

        delete profile;
        profile = 0;
    }

    return success;
}

bool ProfileManager::setSyncSchedule(QString aProfileId , QString aScheduleAsXml)
{
    FUNCTION_CALL_TRACE;
    bool status = false;
    SyncProfile *profile = syncProfile(aProfileId);
    if (profile)
    {
        profile->setSyncType(SyncProfile::SYNC_SCHEDULED);
        QDomDocument doc;
        if(doc.setContent(aScheduleAsXml,true)) {
            SyncSchedule schedule(doc.documentElement());
            profile->setSyncSchedule(schedule);
            updateProfile(*profile);
            status = true;
        }
        delete profile;
        profile = NULL;
    } else {
        LOG_WARNING("Invalid Profile Supplied");
    }
    return status;
}

bool ProfileManagerPrivate::parseFile(const QString &aPath, QDomDocument &aDoc)
{
    //FUNCTION_CALL_TRACE;

    bool parsingOk = false;

    if (QFile::exists(aPath))
    {
        QFile file(aPath);

        if (file.open(QIODevice::ReadOnly))
        {
            parsingOk = aDoc.setContent(&file);
            file.close();

            if (!parsingOk)
            {
                LOG_WARNING("Failed to parse profile XML: " << aPath);
            }
        }
        else {
            LOG_WARNING("Failed to open profile file for reading:" << aPath);
        }
    }
    else
    {
        LOG_WARNING("Profile file not found:" << aPath);
    }



    return parsingOk;
}

QDomDocument ProfileManagerPrivate::constructProfileDocument(const Profile &aProfile)
{
    //FUNCTION_CALL_TRACE;

    QDomDocument doc;
    QDomElement root = aProfile.toXml(doc);

    if (root.isNull())
    {
        LOG_WARNING("Failed to convert profile to XML");
    }
    else
    {
        QDomProcessingInstruction xmlHeading =
                doc.createProcessingInstruction("xml",
                        "version=\"1.0\" encoding=\"UTF-8\"");

        doc.appendChild(xmlHeading);
        doc.appendChild(root);
    }

    return doc;
}

bool ProfileManagerPrivate::writeProfileFile(const QString &aProfilePath,
        const QDomDocument &aDoc)
{
    //FUNCTION_CALL_TRACE;

    QFile file(aProfilePath);
    bool profileWritten = false;

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream outputStream(&file);
        outputStream << aDoc.toString(PROFILE_INDENT);
        file.close();
        profileWritten = true;
    }
    else
    {
        LOG_WARNING("Failed to open profile file for writing:" << aProfilePath);
        profileWritten = false;
    }

    return profileWritten;
}

void ProfileManagerPrivate::restoreBackupIfFound(const QString &aProfilePath,
        const QString &aBackupPath)
{
    //FUNCTION_CALL_TRACE;

    if (QFile::exists(aBackupPath))
    {
        LOG_WARNING("Profile backup file found. The actual profile may be corrupted.");

        QDomDocument doc;
        if (parseFile(aBackupPath, doc))
        {
            LOG_DEBUG("Restoring profile from backup");
            QFile::remove(aProfilePath);
            QFile::copy(aBackupPath, aProfilePath);
        }
        else
        {
            LOG_WARNING("Failed to parse backup file");
            LOG_DEBUG("Removing backup file");
            QFile::remove(aBackupPath);
        }
    }
}

bool ProfileManagerPrivate::createBackup(const QString &aProfilePath,
        const QString &aBackupPath)
{
    FUNCTION_CALL_TRACE;
    return QFile::copy(aProfilePath, aBackupPath);
}

QString ProfileManagerPrivate::findProfileFile(const QString &aName, const QString &aType)
{
    QString fileName = aType + QDir::separator() + aName + FORMAT_EXT;
    QString primaryPath = iPrimaryPath + QDir::separator() + fileName;
    QString secondaryPath = iSecondaryPath + QDir::separator() + fileName;

    if (QFile::exists(primaryPath))
    {
        return primaryPath;
    }
    else if (!QFile::exists(secondaryPath))
    {
        return primaryPath;
    }
    else
    {
        return secondaryPath;
    }
}

// this function checks to see if its a new profile or an
// existing profile being modified under $Sync::syncCacheDir/profiles directory.
bool ProfileManagerPrivate::profileExists(const QString &aProfileId ,const QString &aType)
{
    QString profileFile = iPrimaryPath + QDir::separator() + aType + QDir::separator() + aProfileId + FORMAT_EXT;
    LOG_DEBUG("profileFile:" << profileFile);
    return QFile::exists(profileFile);
}

void ProfileManager::addRetriesInfo(const SyncProfile* profile)
{
    FUNCTION_CALL_TRACE;
    if(profile)
    {
        if(profile->hasRetries() && !iSyncRetriesInfo.contains(profile->name()))
        {
            LOG_DEBUG("syncretries : retries info present for profile" << profile->name());
            iSyncRetriesInfo[profile->name()] = profile->retryIntervals();
        }
    }
}

QDateTime ProfileManager::getNextRetryInterval(const SyncProfile* aProfile)
{
    FUNCTION_CALL_TRACE;
    QDateTime nextRetryInterval;
    if(aProfile &&
       iSyncRetriesInfo.contains(aProfile->name()) &&
       !iSyncRetriesInfo[aProfile->name()].isEmpty())
    {
       quint32 mins = iSyncRetriesInfo[aProfile->name()].takeFirst();
       nextRetryInterval = QDateTime::currentDateTime().addSecs(mins * 60); 
       LOG_DEBUG("syncretries : retry for profile" << aProfile->name() << "in" << mins <<"minutes");
       LOG_DEBUG("syncretries :" << iSyncRetriesInfo[aProfile->name()].count() <<"attempts remain");
    }
    return nextRetryInterval;
}

void ProfileManager::retriesDone(const QString& aProfileName)
{
    FUNCTION_CALL_TRACE;
    if(iSyncRetriesInfo.contains(aProfileName))
    {
       iSyncRetriesInfo.remove(aProfileName);
       LOG_DEBUG("syncretries : retry success for" << aProfileName);
    }
}
