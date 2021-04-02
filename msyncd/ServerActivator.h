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

#ifndef SERVERACTIVATOR_H_
#define SERVERACTIVATOR_H_

#include "TransportTracker.h"
#include "ProfileManager.h"
#include <QObject>
#include <QMap>
#include <QString>

namespace Buteo {
    
/*! \brief Keeps track of which server plug-ins should be enabled
 *
 * Finds out from server profiles which transports each server plug-in uses and
 * concludes the enabled/disabled state based on the transport states. A server
 * is enabled if any of the transports it uses is enabled or if there are any
 * references left to the server. Each matching and enabled transport creates
 * one reference to the server. References can be added and removed also
 * externally, so that a server won't become disabled if it is still in use but
 * all its transports are disabled. A signal is emitted when a server is enabled
 * or disabled. This class does not control the actual server plug-ins, it only
 * gives information about when a server plug-in should be enabled or disabled.
 */
class ServerActivator : public QObject
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * @param aProfileManager Profile manager for accessing server profiles
     * @param aTransportTracker Transport tracker
     * @param aParent Parent object
     */
    ServerActivator(ProfileManager &aProfileManager,
        TransportTracker &aTransportTracker, QObject *aParent = 0);

    //! \brief Destructor
    virtual ~ServerActivator();

    /*! \brief Adds a reference to the given server
     *
     * If the reference count was zero, emits the serverEnabled signal, if the
     * emit signal paramater is true
     * @param aServerName Server profile name
     * @param emitSignal Controls the emission of the serverEnabled signal
     * @return Number of references after the addition
     */
    int addRef(const QString &aServerName, bool emitSignal = true);

    /*! \brief Removes a reference from the given server
     *
     * If the reference count reaches zero, emits the serverDisabled signal, if the
     * emitSignal parameter is true
     * @param aServerName Server profile name
     * @param emitSignal Controls the emission of the serverDisabled signal
     * @return Number of references after the remove
     */
    int removeRef(const QString &aServerName, bool emitSignal = true);

    /*! \brief Gets the list of enabled server
     *
     * @return Server profile names
     */
    QStringList enabledServers() const;

signals:

    /*! \brief Signal emitted when a server should be enabled
     *
     * @param aServerName Server profile name
     */
    void serverEnabled(const QString &aServerName);

    /*! \brief Signal emitted when a server should be disabled
     *
     * @param aServerName Server profile name
     */
    void serverDisabled(const QString &aServerName);

public slots:

    /*! \brief Called when transport state changes
     *
     * @param aType Connectivity type
     * @param aState New state
     */
    void onConnectivityStateChanged(Sync::ConnectivityType aType, bool aState);

private:

    QList<Sync::ConnectivityType> transportsFromProfile(const Profile *aProfile);

    ProfileManager &iProfileManager;

    TransportTracker &iTransportTracker;

    struct ServerData {
        ServerData() : iRefCount(0) { }
        QList<Sync::ConnectivityType> iTransports;
        int iRefCount;
    };

    QMap<QString, ServerData> iServers;

#ifdef SYNCFW_UNIT_TESTS
    friend class ServerActivatorTest;
#endif

};

}

#endif /* SERVERACTIVATOR_H_ */
