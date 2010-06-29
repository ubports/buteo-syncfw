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
        /*! The alarm inventory constructor */
        SyncAlarmInventory();

        /*! The alarm inventory destructor */
        ~SyncAlarmInventory();

        /*! Initialize the alarms database */
        bool init();

        // FIXME! Change int to long for alarmId
        /*! Method to add an alarm */
        int  addAlarm(QDateTime alarmTime);

        /*! Method to remove an alarm */
        bool removeAlarm(int alarmId);

    signals:
        /* Signal triggered when an alarm expired */
        void triggerAlarm(int alarmId);

    private:
        /* Deletes the alarm from DB */
        bool deleteAlarmFromDb( int alarmName );

        /* Method to add an alarm to the database */
        int addAlarmToDb( QDateTime timeStamp );

        /* Method to fetch the database handle */
        QSqlDatabase*  getDbHandle();

        /* Timer object to keep tracke of alarm timers */
        QTimer*        timer;

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
