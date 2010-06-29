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


#ifndef QDBUSABSTRACTADAPTORSYNCSCHEDULERDBUSADAPTOR_H
#define QDBUSABSTRACTADAPTORSYNCSCHEDULERDBUSADAPTOR_H

#include <QtDBus/QtDBus>

namespace Buteo {
    
class SyncSchedulerDBusAdaptor: public QDBusAbstractAdaptor{
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.msyncd.scheduler")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.nokia.msyncd.scheduler\" >\n"
"    <method name=\"triggerAlarm\" >\n"
"      <arg direction=\"in\" type=\"i\" name=\"aAlarmID\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"  </interface>\n"
        "")
            
public:
    /**
     * \brief Constructor
     * @param aParent A pointer to parent object
     */
    SyncSchedulerDBusAdaptor(QObject* aParent);

    /**
     * \brief Destructor
     */
    virtual ~SyncSchedulerDBusAdaptor();
    
public Q_SLOTS:
    
    /**
     * \brief Called when an alarm event is triggered by the alarm daemon
     * @param aAlarmID 
     */
    Q_NOREPLY void triggerAlarm(int aAlarmID);
};

}
#endif
