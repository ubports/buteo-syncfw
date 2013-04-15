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

#ifndef BTHELPER_H
#define BTHELPER_H

#include <QObject>
#include <QtDBus>
#include <QMap>

/*! \brief Strings used for DBus communication with bluetooth daemon are grouped using this structure.
  */
struct BT
{
    /// Destination for Dbus command to bluez
    static const QString BLUEZ_DEST;
    /// Bluez manager interface name
    static const QString BLUEZ_MANAGER_INTERFACE;
    /// Bluez adapter interface name
    static const QString BLUEZ_ADAPTER_INTERFACE;
    /// Bluez Device interface name
    static const QString BLUEZ_DEVICE_INTERFACE;
    /// Method name for retrieving default adapter
    static const QString GET_DEFAULT_ADAPTER;
    /// Method name for finding the device
    static const QString FIND_DEVICE;
    /// Method name for discovering services
    static const QString DISCOVERSERVICES;
    /// Method name for discovering services
    static const QString GETPROPERTIES;
};

/*! \brief Implementation for bluetooth helper utils.
  */
class BtHelper : public QObject
{
    Q_OBJECT

private:
	QString m_deviceAddress;

// Private methods

    /*! \brief Fetches the default adapter path
      */
     QString getDefaultAdapterPath();
    
     /*! \brief Fetches the device path
      * \param defaultAdapterPath Default adapter path
      */
     QString getDevicePath(QString& defaultAdapterPath);
    /* \brief Open the serial port and get file descriptor for the port.
       * \return File descriptor if opening port was success, otherwise -1
       */
public:
    /*! \brief Constructor.
      * \param deviceAddess Bluetooth address of remote device
      * \param parent Parent object
      */
    BtHelper(const QString& deviceAddess, QObject* parent = 0);

    /*! \brief Destructor
      */
    ~BtHelper();

    /*! \brief Fetch the bluetooth services supported by remote device.
      * \param servicesList outparam which will be populated with the services.
      */
     bool getServiceRecords(QList<QString>& servicesList);
    /*! \brief To find if a specific service is supported by remote device.
      * \param servicesList List of remote device sdp records 
      * \param serviceUUID UUID of the service to be connected
      */
     bool isServiceSupported (const QList<QString>& servicesList, const QString& serviceUUID);

    /*! \brief To find remote device BT properties.
      */
     QMap<QString, QVariant> getDeviceProperties();
};

#endif // BTHELPER_H
