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


#ifndef SYNCALARMINVENTORY_H
#define SYNCALARMINVENTORY_H

#include <QObject>
#include <QDateTime>
#include <QtSql>

/*! \brief Class for storing alarms
 *
 * This class stores alarms for scheduled synchronizations. The main elements
 * are the sync time and the alarm id
 */
class SyncAlarmInventory : public QObject
{
    Q_OBJECT

    public:
        /*! The alarm inventory constructor
         * Always Call init()  before using other methods of this class */
        SyncAlarmInventory();

        /*! The alarm inventory destructor */
        ~SyncAlarmInventory();

        /*! \brief Creates and Initialize the alarms database. also Creates the timers
         * Please call this function to make sure the database is initialised properly
         * @return - status of the initialisation
         */
        bool init();

        /*! \brief Method to add an alarm
         *
         * @param alarmTime - time of the alarm as QDateTime
         * @return id of the alarm if alarm was added successfully. else 0
         */
        int  addAlarm(QDateTime alarmTime);

        /*! Method to remove an alarm
         *
         * @param alarmId - id of the alarm to remove
         * @return status of the remove
         */
        bool removeAlarm(int alarmId);

        /*! Method to remove all alarms
         *
         */
        void removeAllAlarms();

    signals:
        /*! \brief Signal triggered when an alarm expired
         * @param alarmId  - id of the alarm that got triggered.
         * */
        void triggerAlarm(int alarmId);

    private:
        /* Deletes the alarm from DB */
        bool deleteAlarmFromDb( int alarmName );

        /* Method to add an alarm to the database */
        int addAlarmToDb( QDateTime timeStamp );

        /* Method to fetch the database handle */
        QSqlDatabase*  getDbHandle();

        /* Timer object to keep tracke of alarm timers */
        QTimer*        iTimer;

        /* Current alarm that is under work */
        int            currentAlarm;

        /* Number of times that the alarm triggers */
        int            triggerCount;

        /* Database handle */
        QSqlDatabase   iDbHandle;

        /* Database connection name */
        QString        iConnectionName;

    private slots:
        /*! Slot used whenever the timer object expires */
        void timerTriggered();
};

#endif
