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
#ifndef SYNCSCHEDULE_H
#define SYNCSCHEDULE_H

#include <QTime>
#include <QSet>

class QDomDocument;
class QDomElement;

namespace Buteo {

class SyncSchedulePrivate;
class SyncScheduleTest;

typedef QSet<int> DaySet;

const QString SYNC_SCHEDULE_ENABLED_KEY_BOOL("scheduler/schedule_enabled");
const QString SYNC_SCHEDULE_PEAK_ENABLED_KEY_BOOL("scheduler/schedule_peak_enabled");
const QString SYNC_SCHEDULE_OFFPEAK_ENABLED_KEY_BOOL("scheduler/schedule_offpeak_enabled");
const QString SYNC_SCHEDULE_PEAK_DAYS_KEY_INT ("scheduler/schedule_peak_days");
const QString SYNC_SCHEDULE_PEAK_START_TIME_KEY_INT ("scheduler/schedule_peak_start_time");
const QString SYNC_SCHEDULE_PEAK_END_TIME_KEY_INT ("scheduler/schedule_peak_end_time");
const QString SYNC_SCHEDULE_PEAK_SCHEDULE_KEY_INT ("scheduler/schedule_peak");
const QString SYNC_SCHEDULE_OFFPEAK_SCHEDULE_KEY_INT ("scheduler/schedule_off_peak");

/*! \brief Class for handling sync schedule settings.
 *
 */
class SyncSchedule
{
public:
    /*! \brief Constructs an empty schedule.
     *
     */
    SyncSchedule();

    /*! \brief Copy constructor.
     *
     * \param aSource Copy source.
     */
    SyncSchedule(const SyncSchedule &aSource);

    /*! \brief Constructs sync schedule from XML.
     *
     * \param aRoot Root element of the XML representation.
     */
    explicit SyncSchedule(const QDomElement &aRoot);

    /*! \brief Destructor.
     */
    ~SyncSchedule();

    /*! \brief Assignment operator.
     *
     * \param aRhs Source
     */
    SyncSchedule& operator=(const SyncSchedule &aRhs);

    /*! \brief Equal to operator.
     *
     * \param aRhs Source
     */
    bool operator==(const SyncSchedule &aRhs);
    
    /*! \brief Exports the sync schedule to XML.
     *
     * \param aDoc Parent document for the created XML elements.
     * \return Root element of the created XML.
     */
    QDomElement toXml(QDomDocument &aDoc) const;

	/*! \brief Exports the sync schedule to QString.
     *
     * \return return the Schedule as xml formatted string
     */
    QString toString() const;

    /*! \brief Gets the enabled week days of the sync schedule.
     *
     * The returned set contains the numbers of enabled week days.
     * Day numbers are defined in Qt::DayOfWeek.
     * \return Set of week day numbers.
     */
    DaySet days() const;

    /*! \brief Sets the enabled week days.
     *
     * \param aDays Set of enabled week days.
     */
    void setDays(const DaySet &aDays);

    /*! \brief Gets the exact time set in sync schedule.
     *
     * \return Sync time. Null object if time has not been defined.
     */
    QTime time() const;

    /*! \brief Sets the exact time for sync.
     *
     * Set to null object QTime() to disable syncing at exact time.
     * \param aTime Sync time.
     */
    void setTime(const QTime &aTime);


    /*! \brief Sets scheduled config time.
    *
    * \param QDateTime Sync time.
    */
    void setScheduleConfiguredTime(const QDateTime &aDateTime);

    /*! \brief To get the scheduled config time.
    *
    *   \return QDateTime Sync time.
    */
    QDateTime scheduleConfiguredTime();

    /*! \brief Gets sync interval in minutes.
     *
     * \return Interval.
     */
    unsigned interval() const;

    /*! \brief Sets sync interval in minutes.
     *
     * Set to zero to disable syncing with intervals.
     * \param aInterval Interval.
     */
    void setInterval(unsigned aInterval);

    /*! \brief Checks if normal schedule is obeyed.
     *
     * \return Normal schedule is obeyed. Return value false corresponds to "manual" mode.
     */
    bool scheduleEnabled() const;

    /*! \brief Sets if normal schedule is to be obeyed.
     *
     * \param aEnabled Specify ifnormal scheduling hours enabled. If set to false, corresponds to "manual" mode.
     */
    void setScheduleEnabled(bool aEnabled);


    // ============== RUSH HOUR SETTINGS ============================


    /*! \brief Checks if rush hour schedule is to be obeyed.
     *
     * \return True if rush hour schedule is to be used. False, if rush hour scheduling is off (i.e. manual mode).
     */
    bool rushEnabled() const;

    /*! \brief Sets rush hour schedule is to be obeyed.
     *
     * \param aEnabled If set to false, corresponds to rush hour scheduling off, i.e. "manual" sync.
     */
    void setRushEnabled(bool aEnabled);

    /*! \brief Gets days enabled for rush hours.
     *
     * \return Set of days enabled for rush hours.
     */
    DaySet rushDays() const;

    /*! \brief Sets days enabled for rush hours.
     *
     * \param aDays Enabled days for rush hours.
     */
    void setRushDays(const DaySet &aDays);

    /*! \brief Gets begin time of rush hours.
     *
     * \return Begin time.
     */
    QTime rushBegin() const;

    /*! \brief Gets end time of rush hours.
     *
     * \return End time.
     */
    QTime rushEnd() const;

    /*! \brief Sets begin and end times of rush hours.
     *
     * \param aBegin Begin time.
     * \param aEnd End time.
     */
    void setRushTime(const QTime &aBegin, const QTime &aEnd);

    /*! \brief Gets sync interval for rush hours.
     *
     * \return Interval in minutes.
     */
    unsigned rushInterval() const;

    /*! \brief Sets sync interval for rush hours.
     *
     * \param aInterval Interval.
     */
    void setRushInterval(unsigned aInterval);

    /*! \brief Gets next sync time based on the sync schedule settings.
     *
     * \param aPrevSync Previous sync time.
     * \param aPrevious sync time.
     * \return Next sync time. Null object if schedule is not defined.
     */
    QDateTime nextSyncTime(const QDateTime &aPrevSync) const;

private:

    SyncSchedulePrivate* d_ptr;

#ifdef SYNCFW_UNIT_TESTS
    friend class SyncScheduleTest;
#endif

};

}

#endif // SYNCSCHEDULE_H
