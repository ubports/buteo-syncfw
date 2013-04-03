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
#ifndef SYNCSCHEDULE_P_H
#define SYNCSCHEDULE_P_H

#include <QDateTime>
#include <QString>

namespace Buteo {
    
//! Private implementation class for SyncSchedule.
class SyncSchedulePrivate
{
public:
	/*! \brief Constructor
	 *
	 */
    SyncSchedulePrivate();

    /*! \brief Copy Constructor
     *
     */
    SyncSchedulePrivate(const SyncSchedulePrivate &aSource);

    /*! \brief Parses week day numbers from a string.
     *
     * \param aDays String containing the week day numbers.
     * \return Set of week day numbers.
     */
    DaySet parseDays(const QString &aDays) const;

    /*! \brief Creates a string from a set of week day numbers.
     *
     * \param aDays Set of week day numbers.
     * \return String of week day numbers.
     */
    QString createDays(const DaySet &aDays) const;

    /*! \brief Adjusts given date to be in the set of given week days.
     *
     * Day is increased until the week day is contained in the given set of
     * week day numbers.
     * \param aTime Date/time to adjust.
     * \param aDays Set of enabled week day numbers.
     * \return Was day adjusted to a valid day. If the week day was already in
     *  the set of given week days, this function returns false. If the day
     *  set does not contain any valid days, this function sets aTime to null
     *  object and returns false.
     */
    bool adjustDate(QDateTime &aTime, const DaySet &aDays) const;

    /*! \brief Checks if the given date/time is inside rush hours.
     *
     * \param aTime Date/time to check.
     * \return True if in rush hours.
     */
    bool isRush(const QDateTime &aTime) const;

    //! Number of Days before the next sync starts
    DaySet iDays;

    //! Sync Time
    QTime iTime;

    //! sync schedule configure time for intial update
    QDateTime iScheduleConfiguredTime;

    //! Time interval
    unsigned iInterval;

    bool iEnabled;

    // ============ RUSH HOUR SETTINGS =========== 

    //! indicates the schedule for rush hour - days
    DaySet iRushDays;

    //! indicates the schedule for rush hour start
    QTime iRushBegin;

    //! indicates the schedule for rush hour end
    QTime iRushEnd;

    //! Rush Hour Time interval
    unsigned iRushInterval;

    //! Indicates if Rush Hour is Enabled
    bool iRushEnabled;
};

}

#endif // SYNCSCHEDULE_P_H
