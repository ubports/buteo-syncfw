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

#include "SyncAlarmInventory.h"

#include <QTimer>
#include <QObject>
#include <QSettings>
#include <QDebug>

const QString ALARM_CONNECTION_NAME( "alarms" );

// Use this to calculate the number of times a timer has to be triggered. 
// For now, we just divide the next trigger time by 2 and use that value
const int TRIGGER_COUNT = 2;

SyncAlarmInventory::SyncAlarmInventory()
{
    static unsigned connectionNumber = 0;
    iConnectionName = ALARM_CONNECTION_NAME + QString::number( connectionNumber++ );
    iDbHandle = QSqlDatabase::addDatabase( "QSQLITE", iConnectionName );

    QString path( QDir::home().path() );
    path.append( QDir::separator()).append( "alarms.db.sqlite" );
    path = QDir::toNativeSeparators( path );

    iDbHandle.setDatabaseName( path );

    // Create the timer object
    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(timerTriggered()) );
    currentAlarm = 0;
    // Leave the open db to addAlarm
}

bool SyncAlarmInventory::init()
{
    // The database is already defined in the constructor. Create a database handle
    // and also the alarm table
    if (!iDbHandle.open()) {
        qDebug() << "Unable to initialize Alarms sqlite database.";
        return false;
    }

    // Create the alarms table
    const QString createTableQuery( "CREATE TABLE IF NOT EXISTS alarms(alarmid INTEGER PRIMARY KEY AUTOINCREMENT, synctime DATETIME)" );
    QSqlQuery query( createTableQuery, iDbHandle );
    if ( query.exec() )
        return true;
    else {
        qDebug() << "Unable to create alarms table";
        return false;
    }

}

SyncAlarmInventory::~SyncAlarmInventory()
{
    iDbHandle.close();
    iDbHandle = QSqlDatabase();
    QSqlDatabase::removeDatabase( iConnectionName );
    
    if (timer) {
        timer->stop();
        delete timer;
	    timer = 0;
    }

    // TODO: Cleanup other objects
}

int SyncAlarmInventory::addAlarm( QDateTime alarmDate )
{
    // Check if alarmDate < QDateTime::currentDateTime()
    if ( QDateTime::currentDateTime().secsTo(alarmDate) < 0 )
        return 0;

    // Store the alarm 

    int alarmId = 0;
    if ( (alarmId = addAlarmToDb(alarmDate)) == 0 ) {
        qDebug() << "Unable to create alarm:"+alarmId ;
        // Note: Even incase of an already existing profile, false is returned by the query
        // There is no way to detect a record insertion from an already existing alarm
        return 0;
    }

    // Select all the alarms from the db sorted by alarm time
    QSqlQuery selectQuery( iDbHandle );
    qDebug() << "Fetching alarms from DB";
    if ( selectQuery.exec("SELECT alarmid,synctime FROM alarms ORDER BY synctime ASC") ) {
        if ( selectQuery.first() ) {
            int newAlarm = selectQuery.value(0).toInt();
            QDateTime alarmTime = selectQuery.value(1).toDateTime();
            qDebug() << "Found alarm:" << newAlarm << ", time:" << alarmTime;

            // If the newAlarm != currentAlarm that is fetched from DB, stop the
            // previous timer
            if ( (currentAlarm != 0) && (newAlarm != currentAlarm) ) {
                qDebug() << "New alarm:" << newAlarm;
                if ( !timer->isActive() )
                    timer->stop();
            }

            // This is a new alarm. Set the timer for the alarm
            currentAlarm = newAlarm;
            QDateTime now = QDateTime::currentDateTime();
            int timerInterval = (now.secsTo( alarmTime ) / TRIGGER_COUNT) * 1000;  // time interval in millisec
            qDebug() << "Setting timer for:" << currentAlarm << " with timerInterval:" << timerInterval << ",now:" << now << ",alarmTime:" << alarmTime;
            triggerCount = TRIGGER_COUNT;
            timer->setInterval( timerInterval );
            timer->start();
        }
    }
    return alarmId;
}

bool SyncAlarmInventory::removeAlarm(int alarmId)
{
    if( alarmId <= 0 ) return false;
    deleteAlarmFromDb( alarmId );
    return true;
}

void SyncAlarmInventory::timerTriggered()
{
    // Decrement the alarm counter
    triggerCount--;
    qDebug() << "Timer triggered for alarm:" << currentAlarm << " with count:" << triggerCount;

    // Alarm expired. Trigger the alarm and delete it from DB and set the alarm for the next one
    if (triggerCount == 0) {
        emit triggerAlarm(currentAlarm);
        qDebug() << "Triggered alarm:" << currentAlarm << ", at:" << QDateTime::currentDateTime();

        // Delete the alarm from DB
        if ( !deleteAlarmFromDb(currentAlarm) ) {
            qDebug() << "Unable to delete alarm:" << currentAlarm << " from DB";
        }
        timer->stop();
        currentAlarm = 0;

        // Set the new alarm timer
        // Select all the alarms from the db sorted by alarm time
        QSqlQuery selectQuery( iDbHandle );
        if ( selectQuery.exec("SELECT alarmid,synctime FROM alarms ORDER BY synctime ASC") ) {
            if ( selectQuery.first() ) {
                currentAlarm = selectQuery.value(0).toInt();
                QDateTime alarmTime = selectQuery.value(1).toDateTime();

                // Set the timer for the alarm
                QDateTime now = QDateTime::currentDateTime();
                int timerInterval = (now.secsTo( alarmTime ) / TRIGGER_COUNT) * 1000;  // time interval in millisec
                triggerCount = TRIGGER_COUNT;
                timer->setInterval( timerInterval );
                timer->start();
            }
        }
    }
}

bool SyncAlarmInventory::deleteAlarmFromDb( int alarmId )
{
    QSqlQuery removeQuery ( iDbHandle );
    removeQuery.prepare( "DELETE FROM alarms WHERE alarmid=:alarmid" );
    removeQuery.bindValue( ":alarmid", alarmId );

    if ( !removeQuery.exec() )
        return false;
    else {
        qDebug() << "alarmId:" << alarmId << " deleted from db";
        return true;
    }
}

int SyncAlarmInventory::addAlarmToDb( QDateTime timeStamp )
{
    QSqlQuery insertQuery( iDbHandle );
    insertQuery.prepare( "INSERT INTO alarms(synctime) VALUES(:synctime)" );
    insertQuery.bindValue( ":synctime", timeStamp );

    if ( insertQuery.exec() )
        return insertQuery.lastInsertId().toInt();
    else
        return 0;
}
