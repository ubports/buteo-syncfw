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
#ifndef IPHEARTBEAT_H
#define IPHEARTBEAT_H

#include <QObject>
#include <QMap>
#include <QSocketNotifier>
extern "C" {
    #include "iphbd/libiphb.h"
}

namespace Buteo {

/// \brief IPHeartBeat implementation.
///
/// This class manages heart beats for different profiles.
class IPHeartBeat : public QObject
{
    Q_OBJECT

    /// \brief Internal structure to hold profile-iphb stucture-notifier map
    struct BeatStruct
    {
        int sockfd;
        QSocketNotifier* sockNotifier;
        iphb_t iphbHandle;
    };

public:
    /*! \brief Constructor.
     *  \param aParent Parent object.
     */
    IPHeartBeat(QObject *aParent);

    /**
     * \brief Destructor
     */
    virtual ~IPHeartBeat();

    /*! \brief Schedules a heartbeat for this profile between minWaitTime and maxWaitTime.
     *
     * The beat will be generated between minWaitTime and maxWaitTime seconds
     * \param aProfName Name of the profile.
     * \param aMinWaitTime Minimum wait time in seconds.
     * \param aMaxWaitTime Minimum wait time in seconds.
     * \return Success indicator.
     */
    bool setHeartBeat(const QString& aProfName, ushort aMinWaitTime, ushort aMaxWaitTime);

    /*! \brief Removes heart beat waiting for a profile.
     *
     * \param aProfName Name of the profile.
     */
    void removeWait(const QString& aProfName);

    /*! \brief Removes heart beat waiting for all profiles.
     */
    void removeAllWaits();

signals:

    /*! \brief This signal will be emitted when a heartbeat for particular profile is triggered.
     *
     * \param aProfName Name of the profile for which heart beat is triggered.
     */
    void onHeartBeat(QString aProfName);


private slots:

    /*! \brief This signal will be emitted when a socket descriptor gets an event notification.
     *
     * \param aSockFd Socket descriptor who got the event.
     */
    void internalBeatTriggered(int aSockFd);


private:

    /*! \brief Finds the name of the profile which uses particular file descriptor
     *
     * \param aSockFd Socket descriptor.
     * \param aProfName name of the profile.
     * \return true if profile name found, otherwise false
     */
    bool getProfNameFromFd(int aSockFd, QString& aProfName);


private:

    ///Map of structures waiting for heart beat
    QMap<QString, BeatStruct> iBeatsWaiting;

#ifdef SYNCFW_UNIT_TESTS
    friend class IPHeartBeatTest;
#endif

};

}

#endif
