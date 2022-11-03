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
#include <QDebug>
#include <LogMacros.h>

#include "BtHelper.h"


const QString BT::BLUEZ_DEST = "org.bluez";
const QString BT::BLUEZ_MANAGER_INTERFACE = "org.bluez.Manager";
const QString BT::BLUEZ_ADAPTER_INTERFACE = "org.bluez.Adapter";
const QString BT::BLUEZ_DEVICE_INTERFACE = "org.bluez.Device";
const QString BT::GET_DEFAULT_ADAPTER = "DefaultAdapter";
const QString BT::FIND_DEVICE = "FindDevice";
const QString BT::DISCOVERSERVICES = "DiscoverServices";
const QString BT::GETPROPERTIES = "GetProperties";


BtHelper::BtHelper(const QString& deviceAddress,
                       QObject* parent) : QObject(parent)
{
    m_deviceAddress         = deviceAddress;
}

BtHelper::~BtHelper()
{
  LOG_DEBUG ("");    
}



bool BtHelper::isServiceSupported (const QList<QString>& servicesList, const QString& serviceUUID)
{
   LOG_DEBUG ("isServiceSupported");
   foreach (QString service, servicesList) {
  	//LOG_DEBUG ("Record : "  << service);
	if (service.contains(serviceUUID)){   
   		LOG_DEBUG ("Service found " << serviceUUID);
		return true;
	}
   }
   return false;
}

QString BtHelper::getDefaultAdapterPath()
{
    LOG_DEBUG ("getDefaultAdapterPath");
    
    QDBusInterface managerInterface( BT::BLUEZ_DEST, "/",
                                     BT::BLUEZ_MANAGER_INTERFACE,
                                     QDBusConnection::systemBus() );

    if( !managerInterface.isValid() ) {
        LOG_DEBUG ("Manager interface is invalid");
        return QString();
    }

    QDBusReply<QDBusObjectPath> pathReply = managerInterface.call(BT::GET_DEFAULT_ADAPTER);

    if( !pathReply.isValid() ) {
        LOG_DEBUG ("Not able to get the adapter path");
        return QString();
    }
    return pathReply.value().path();
}

QString BtHelper::getDevicePath(QString &defaultAdapterPath)
{
    if (defaultAdapterPath.isEmpty()) {
    	LOG_DEBUG ( "Adapter path is empty");
	return QString();	
    }	

    QDBusInterface adapterInterface(BT::BLUEZ_DEST, defaultAdapterPath,
                                    BT::BLUEZ_ADAPTER_INTERFACE, QDBusConnection::systemBus() );
    if( !adapterInterface.isValid() ) {
        LOG_DEBUG ( "Adapter interface is invalid");
        return QString();
    }

    QDBusReply<QDBusObjectPath> pathReply = adapterInterface.call(BT::FIND_DEVICE, m_deviceAddress );

    if( !pathReply.isValid() ) {
        LOG_DEBUG ( "Not able to find the BT device");
        return QString();
    }
    return pathReply.value().path();
}

bool BtHelper::getServiceRecords(QList<QString>& servicesList)
{
    LOG_DEBUG ( "getServiceRecords()");

    QString defaultAdapterPath = getDefaultAdapterPath();
    LOG_DEBUG ( "Adapter path = " << defaultAdapterPath) ;

    QString devicePath = getDevicePath(defaultAdapterPath);
    if (devicePath.isEmpty())
	    return false;
    LOG_DEBUG ( "Device path =" << devicePath);

    QDBusInterface deviceInterface(BT::BLUEZ_DEST, devicePath,
                                   BT::BLUEZ_DEVICE_INTERFACE,
                                   QDBusConnection::systemBus() );
    if( deviceInterface.isValid() == false ) {
        LOG_DEBUG ("Device interface is not valid");
        return false;
    }

   QDBusMessage message = deviceInterface.call(BT::DISCOVERSERVICES, QString());
   QDBusArgument reply = QDBusReply<QDBusArgument>(message).value();
   QMap<uint, QString> mapVal;
   reply >> mapVal;  
   servicesList = mapVal.values();
   if (servicesList.size() > 0)
	   return true;
   return false;
}

QMap<QString, QVariant> BtHelper::getDeviceProperties()
{
    LOG_DEBUG ( "getDeviceProperties");

    QMap<QString, QVariant> mapVal;
    QString defaultAdapterPath = getDefaultAdapterPath();
    LOG_DEBUG ( "Adapter path = " << defaultAdapterPath) ;

    QString devicePath = getDevicePath(defaultAdapterPath);
    if (devicePath.isEmpty())
	    return mapVal;
    LOG_DEBUG ( "Device path =" << devicePath);

    QDBusInterface deviceInterface(BT::BLUEZ_DEST, devicePath,
                                   BT::BLUEZ_DEVICE_INTERFACE,
                                   QDBusConnection::systemBus() );
    if( deviceInterface.isValid() == false ) {
        LOG_DEBUG ("Device interface is not valid");
        return mapVal;
    }

   QDBusMessage message = deviceInterface.call(BT::GETPROPERTIES);
   QDBusArgument reply = QDBusReply<QDBusArgument>(message).value();
   reply >> mapVal;  
   return mapVal;
}
