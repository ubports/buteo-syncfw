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


#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include "SyncProfile.h"
#include "Profile.h"
#include <QList>

namespace Buteo {

class ProfileManagerPrivate;
    
/*! \brief
 * ProfileManager is responsible for storing and retrieving the profiles.
 *
 * It also constructs top level profiles by loading and merging all referenced
 * sub-profiles. The ProfileManager hides the actual storage from the user, so
 * that it makes no difference if the profiles are stored to simple XML-files
 * or to a database. Profiles can be queried by name and type.
 */
class ProfileManager
{
public:
    static const QString DEFAULT_PRIMARY_PROFILE_PATH;
    static const QString DEFAULT_SECONDARY_PROFILE_PATH;

    //! \brief Search criteria for finding profiles.
    struct SearchCriteria
    {
        enum Type
        {
            //! Sub-profile (and key) exists.
            EXISTS,

            //! Sub-profile (or key) does not exist.
            NOT_EXISTS,

            //! Key value is equal.
            EQUAL,

            //! Key value is not equal.
            NOT_EQUAL
        };

        //! \brief Constructor.
        SearchCriteria();

        //! \brief Copy constructor.
        SearchCriteria(const SearchCriteria &aSource);

        //! Search criteria type.
        Type iType;

        //! Sub-profile name. If this is empty but profile type is given,
        //! matching is tried with each sub-profile of correct type. If both
        //! profile name and type are empty, mathing is done with keys of the
        //! main profile.
        QString iSubProfileName;

        //! Sub-profile type. If this is empty but profile name is given,
        //! matching is done with the first sub-profile having the correct name
        //! regardless of the type.
        QString iSubProfileType;

        //! Key name. If this is empty, key comparison is not made.
        QString iKey;

        //! Key value. This must be given if criteria type is EQUAL or NOT_EQUAL.
        QString iValue;
    };

    /*! \brief Constructor.
     *
     * \param aPrimaryPath Path where profiles are searched first. Save
     *  operations will write to this location.
     * \param aSecondaryPath Path where a profile is searched for if it is
     *  not found from the primary path. Useful for having default read-only
     *  profiles.
     */
    ProfileManager(const QString &aPrimaryPath = DEFAULT_PRIMARY_PROFILE_PATH,
                   const QString &aSecondaryPath = DEFAULT_SECONDARY_PROFILE_PATH);

    /*! \brief Destructor.
     */
    ~ProfileManager();

    /*! \brief Gets the names of all available profiles with the given type.
     *
     * \param aType Type of the profiles to get.
     * \return The list of profile names.
     */
    QStringList profileNames(const QString &aType);

    /*! \brief Gets a sync profile.
     *
     * Loads and merges also all sub-profiles that are referenced from the
     * main profile. Loads the log of finished synchronization sessions with
     * this profile.
     * \param aName Name of the profile to get.
     * \return The sync profile. NULL if the profile is not found. Caller becomes
     * the owner of the returned object and is responsible of deleting it after
     * use. Changes made to the profile are not saved to the persistent profile
     * storage, unless save function of this class is called.
     */
    SyncProfile *syncProfile(const QString &aName);

    /*! \brief Gets all sync profiles.
     *
     * \return The list of sync profiles. Caller is responsible for deleting
     *  the returned profile objects.
     */
    QList<SyncProfile*> allSyncProfiles();

    /*! \brief Gets all visible sync profiles.
     *
     * Returns all sync profiles that should be visible in sync ui. A profile
     * is visible if it has not been explicitly set as hidden.
     * \return The list of sync profiles. Caller is responsible for deleting
     *  the returned profile objects.
     */
    QList<SyncProfile*> allVisibleSyncProfiles();

    /*! \brief Gets profiles with matching data.
     *
     * \param aSubProfileName Name of a required sub-profile. If this is given,
     *  the sub-profile must exist and key comparison is made with the keys
     *  of the sub-profile.
     * \param aSubProfileType Type of a required sub-profile. If this is given but
     *  sub-profile name is empty, the first sub-profile with matching type is
     *  used in comparison.
     * \param aKey Name of a required key. If this is empty, key comparison is
     *  not made and existance of the sub-profile is enough.
     * \param aValue Value of the required key. If this is empty, any value
     *  is accepted as long as the key itself exists.
     * \return List of matching profiles. Caller is responsible for deleting
     *  the returned profile objects.
     */
    QList<SyncProfile*> getSyncProfilesByData(const QString &aSubProfileName,
        const QString &aSubProfileType,
        const QString &aKey = "", const QString &aValue = "");

    /*! \brief Gets profiles with matching data.
     *
     * \param aCriteria List of criteria to use in the search. Each criterion
     *  in the list has to match for a profile to be returned as a result.
     * \return List of matching profiles. Caller is responsible for deleting
     *  the returned profile objects.
     */
    QList<SyncProfile*> getSyncProfilesByData(
        const QList<SearchCriteria> &aCriteria);

    /*! \brief Gets profiles based on supported storages.
     *
     * Returns all enabled and visible sync profiles of online destinations
     * that support the given storage. Device-to-device sync profiles are not
     * returned.
     * \param aStorageName Name of the storage that must be supported.
     * \param aStorageMustBeEnabled True if the supported storage must be
     *  also enabled. Only enabled storages are included in sync session.
     * \return List of matching profiles. Caller is responsible for deleting
     *  the returned profile objects.
     */
    QList<SyncProfile*> getSyncProfilesByStorage(
        const QString &aStorageName, bool aStorageMustBeEnabled = false);

    /*! \brief Saves a profile to a persistent storage.
     *
     * \param aProfile Profile to save.
     * \return True if saving was successful.
     */
    bool save(const Profile &aProfile);

    /*! \brief Deletes a profile from the persistent storage.
     *
     * \param aName Name of the profile to remove.
     * \param aType Type of the profile to remove.
     * \return Success indicator.
     */
    bool remove(const QString &aName, const QString &aType);

    /*! \brief Expands the given profile.
     *
     * Loads and merges all sub-profiles that are referenced from the main
     * profile.
     * \param aProfile Name of the profile to expand.
     */
    void expand(Profile &aProfile);

    /*! \brief Saves the given synchronization log.
     *
     * \param aLog Log to save.
     * \return True if saving was successful.
     */
    bool saveLog(const SyncLog &aLog);

    /*! \brief Saves the results of a sync session to the log.
     *
     * This is a convenience function that loads the log associated with the
     * given profile, appends the given results to the log and then saves the
     * log.
     * \param aProfileName Name of the profile used in the sync session.
     * \param aResults Results.
     * \return True if saving was successful.
     */
    bool saveSyncResults(QString aProfileName, const SyncResults &aResults);

    /*! \brief Gets a profile.
     *
     * \param aName Name of the profile to get.
     * \param aType Type of the profile to get.
     * \return Pointer to the profile. If the profile is not found, NULL is
     *  returned. Caller is responsible for deleting the returned object.
     *  Changes made to the profile are not saved to profile storage, unless
     *  save function of this class is called
     */
    Profile *profile(const QString &aName, const QString &aType);
     
    /*! \brief Gets a temporary profile (saved if sync is sucessfull).
     *
     * \param destAddress  Address of the remote device bt address/usb .
     * \param saveNewProfile If to save the profile or not (e.g pc suite profile)
     * \return Pointer to the profile. 
     *  Changes made to the profile are not saved to profile storage, unless
     *  save function of this class is called
     */
    SyncProfile *createTempSyncProfile (const QString &btAddress, bool &saveNewProfile);

    /*! \brief Renames a profile, and the associated log too
     *
     * \param aName The old name of the profile
     * \param aNewName The new name for the profile
     * \return Returns true if the rename was successful
     */
    bool rename(const QString &aName, const QString &aNewName);
  
    /*! \brief Enables synced storages in profile
     *
     * \param profile Profile of the remote device
     * \aStorageMap Map of storage names(hcalendar, hcontacts) and if sync 
     * enabled value true/false
     *
     */ 
    void enableStorages (Profile &aProfile, QMap<QString , bool> &aStorageMap); 
   
    /*! \brief Sets remote target in profile
     *
     * \param profile Profile of the remote device
     * \aId remote device id  
     *
     */ 
    void saveRemoteTargetId (Profile &aProfile,const QString& aId);
    
    friend class ProfileManagerTest;

private:
    
    ProfileManager& operator=(const ProfileManager &aRhs);
    
    ProfileManagerPrivate *d_ptr;
};

}

#endif // PROFILEMANAGER_H
