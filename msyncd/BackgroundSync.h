/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Contact: Valerio Valerio <valerio.valerio@jolla.com>
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

#ifndef BACKGROUNDSYNC_H
#define BACKGROUNDSYNC_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <keepalive/backgroundactivity.h>

class BackgroundActivity;

/// \brief BackgroundSync implementation.
///
/// This class manages background syncs for different profiles.
class BackgroundSync : public QObject
{
    Q_OBJECT

    /// \brief Internal structure to hold profile-background activity stucture-notifier map
    struct BActivityStruct
    {
        QString id;
        BackgroundActivity* backgroundActivity;
        BackgroundActivity::Frequency frequency;
    };

     /// \brief Internal structure to hold profile-background activity switch stucture-notifier map
    struct BActivitySwitchStruct
    {
        QString id;
        BackgroundActivity* backgroundActivity;
        QDateTime nextSwitch;
    };

public:
    /*! \brief Constructor.
     *  \param aParent Parent object.
     */
    BackgroundSync(QObject *aParent);

    /**
     * \brief Destructor
     */
    virtual ~BackgroundSync();

    /*! \brief Schedules a background sync for this profile.
     *
     * The beat will be generated between minWaitTime and maxWaitTime seconds
     * \param aProfName Name of the profile.
     * \param seconds Sync frequency in seconds
     * \return Success indicator.
     */

    bool set(const QString& aProfName, int seconds);

    /*! \brief Removes background sync for a profile.
     *
     * \param aProfName Name of the profile.
     */
    bool remove(const QString& aProfName);

    /*! \brief Removes all background syncs for all profiles.
     */
    void removeAll();

    // Sync switch

    /*! \brief Schedules a switch(rush/off-rush) for a background sync running for this profile, the switch should be
     *  added after the background activity.
     *
     * \param aProfName Name of the profile.
     * \param aSwitchTime when the switch will occurs
     * \return Success indicator.
     */
    bool setSwitch(const QString& aProfName, const QDateTime& aSwitchTime);

    /*! \brief Removes  a switch(rush/off-rush) for a profile.
     *
     * \param aProfName Name of the profile.
     */
    bool removeSwitch(const QString& aProfName);

signals:

    /*! \brief This signal will be emitted when a background sync timer for particular profile is triggered.
     *
     * \param aProfName Name of the profile for which background sync timer is triggered.
     */
    void onBackgroundSyncRunning(QString aProfName);

    /*! \brief This signal will be emitted when a switch timer for particular profile is triggered.
     *
     * \param aProfName Name of the profile for which switch timer is triggered.
     */
    void onBackgroundSwitchRunning(const QString& aProfName);

public slots:
    /*! \brief Called when background sync is completed
     *
     * \param aProfName Name of the profile for which background sync is completed.
     */
    void onBackgroundSyncCompleted(QString aProfName);

private slots:

    /*! \brief Called when background sync timer starts running
     */
    void onBackgroundSyncStarted();

    /*! \brief Called when a switch timer starts running
     */
    void onBackgroundSwitchStarted();

private:

    /*! \brief Finds the name of the profile which uses particular background activity
     *
     * \param activityId Id of the background activity
     * \return name of the profile.
     */
    QString getProfNameFromId(const QString activityId) const;

    /*! \brief Returns a valid BackgroundActivity frequency
     *
     * \param seconds Amounth of time for the frequency
     * \return BackgroundActivity frequency
     */

    BackgroundActivity::Frequency frequencyFromSeconds(int seconds);

    // Sync switch

    /*! \brief Removes all scheduled switches(rush/off-rush) for all profiles.
     */
    void removeAllSwitches();

    /*! \brief Finds the name of the profile which uses particular switch activity id
     *
     * \param activityId Id of the switch activity
     * \return name of the profile.
     */
    QString getProfNameFromSwitchId(const QString activityId) const;

private:

    ///Map of structures waiting for background sync
    QMap<QString, BActivityStruct> iScheduledSyncs;

    ///Map of switch timer structures
    QMap<QString, BActivitySwitchStruct> iScheduledSwitch;
};

#endif // BACKGROUNDSYNC_H
