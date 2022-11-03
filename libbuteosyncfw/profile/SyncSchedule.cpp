/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2014-2015 Jolla Ltd
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
#include "SyncSchedule.h"
#include "SyncSchedule_p.h"
#include "ProfileEngineDefs.h"
#include "LogMacros.h"
#include <QDomDocument>
#include <QStringList>

using namespace Buteo;

static const QString DAY_SEPARATOR = ",";

SyncSchedulePrivate::SyncSchedulePrivate()
    :   iInterval(0), iEnabled(false), iRushInterval(0), iRushEnabled(false), iExternalRushEnabled(false)
{
}

SyncSchedulePrivate::SyncSchedulePrivate(const SyncSchedulePrivate &aSource)
:   iDays(aSource.iDays),
    iTime(aSource.iTime),
    iScheduleConfiguredTime(aSource.iScheduleConfiguredTime),
    iInterval(aSource.iInterval),
    iEnabled(aSource.iEnabled),
    iRushDays(aSource.iRushDays),
    iRushBegin(aSource.iRushBegin),
    iRushEnd(aSource.iRushEnd),
    iRushInterval(aSource.iRushInterval),
    iRushEnabled(aSource.iRushEnabled),
    iExternalRushEnabled(aSource.iExternalRushEnabled)
{
}

SyncSchedule::SyncSchedule()
:   d_ptr(new SyncSchedulePrivate())
{
}

SyncSchedule::SyncSchedule(const SyncSchedule &aSource)
:   d_ptr(new SyncSchedulePrivate(*aSource.d_ptr))
{
}

SyncSchedule::SyncSchedule(const QDomElement &aRoot)
:   d_ptr(new SyncSchedulePrivate())
{
    d_ptr->iTime = QTime::fromString(aRoot.attribute(ATTR_TIME), Qt::ISODate);
    d_ptr->iInterval = aRoot.attribute(ATTR_INTERVAL).toUInt();
    d_ptr->iEnabled = (aRoot.attribute(ATTR_ENABLED) == BOOLEAN_TRUE);
    d_ptr->iDays = d_ptr->parseDays(aRoot.attribute(ATTR_DAYS));
    d_ptr->iScheduleConfiguredTime = QDateTime::fromString(aRoot.attribute(ATTR_SYNC_CONFIGURE),Qt::ISODate);

    QDomElement rush = aRoot.firstChildElement(TAG_RUSH);
    if (!rush.isNull())
    {
        d_ptr->iRushEnabled = (rush.attribute(ATTR_ENABLED) == BOOLEAN_TRUE);
        d_ptr->iExternalRushEnabled = (rush.attribute(ATTR_EXTERNAL_SYNC) == BOOLEAN_TRUE);
        d_ptr->iRushInterval = rush.attribute(ATTR_INTERVAL).toUInt();
        d_ptr->iRushBegin = QTime::fromString(rush.attribute(ATTR_BEGIN), Qt::ISODate);
        d_ptr->iRushEnd = QTime::fromString(rush.attribute(ATTR_END), Qt::ISODate);
        d_ptr->iRushDays = d_ptr->parseDays(rush.attribute(ATTR_DAYS));
    }
    else
    {
        d_ptr->iRushEnabled = false;
        d_ptr->iExternalRushEnabled = false;
        d_ptr->iRushInterval = 0;
    }
}

SyncSchedule::~SyncSchedule()
{
    delete d_ptr;
    d_ptr = 0;
}

SyncSchedule& SyncSchedule::operator=(const SyncSchedule &aRhs)
{
    if (&aRhs != this)
    {
        delete d_ptr;
        d_ptr = new SyncSchedulePrivate(*aRhs.d_ptr);
    }

    return *this;
}

bool SyncSchedule::operator==(const SyncSchedule &aRhs)
{
    if (&aRhs == this)
	return true;    
    if (d_ptr->iRushDays != aRhs.d_ptr->iRushDays)
        return false;	    
    else if (d_ptr->iRushBegin != aRhs.d_ptr->iRushBegin)
        return false;	    
    else if (d_ptr->iRushEnd != aRhs.d_ptr->iRushEnd)
        return false;	    
    else if (d_ptr->iRushInterval != aRhs.d_ptr->iRushInterval)    
        return false;	    
    else if (d_ptr->iInterval  != aRhs.d_ptr->iInterval)
        return false;	    
    else if (d_ptr->iEnabled  != aRhs.d_ptr->iEnabled)
        return false;
    else if (d_ptr->iRushEnabled  != aRhs.d_ptr->iRushEnabled) 
        return false;
    else if (d_ptr->iExternalRushEnabled  != aRhs.d_ptr->iExternalRushEnabled)
        return false;

    return true;
}

QDomElement SyncSchedule::toXml(QDomDocument &aDoc) const
{
    QDomElement root = aDoc.createElement(TAG_SCHEDULE);
    root.setAttribute(ATTR_ENABLED, d_ptr->iEnabled ? BOOLEAN_TRUE :
        BOOLEAN_FALSE);
    root.setAttribute(ATTR_TIME, d_ptr->iTime.toString(Qt::ISODate));
    root.setAttribute(ATTR_INTERVAL, QString::number(d_ptr->iInterval));
    root.setAttribute(ATTR_DAYS, d_ptr->createDays(d_ptr->iDays));
    root.setAttribute(ATTR_SYNC_CONFIGURE,d_ptr->iScheduleConfiguredTime.toString(Qt::ISODate));

    QDomElement rush = aDoc.createElement(TAG_RUSH);
    rush.setAttribute(ATTR_ENABLED, d_ptr->iRushEnabled ? BOOLEAN_TRUE :
        BOOLEAN_FALSE);
    rush.setAttribute(ATTR_EXTERNAL_SYNC, d_ptr->iExternalRushEnabled ? BOOLEAN_TRUE :
        BOOLEAN_FALSE);
    rush.setAttribute(ATTR_INTERVAL, QString::number(d_ptr->iRushInterval));
    rush.setAttribute(ATTR_BEGIN, d_ptr->iRushBegin.toString(Qt::ISODate));
    rush.setAttribute(ATTR_END, d_ptr->iRushEnd.toString(Qt::ISODate));
    rush.setAttribute(ATTR_DAYS, d_ptr->createDays(d_ptr->iRushDays));
    root.appendChild(rush);

    return root;
}

QString SyncSchedule::toString() const
{
    QDomDocument doc;
    QDomProcessingInstruction xmlHeading =
        doc.createProcessingInstruction("xml",
        "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlHeading);
    QDomElement root = toXml(doc);
    doc.appendChild(root);

    return doc.toString(PROFILE_INDENT);
}

DaySet SyncSchedule::days() const
{
    return d_ptr->iDays;
}

void SyncSchedule::setDays(const DaySet &aDays)
{
    d_ptr->iDays = aDays;
}

void SyncSchedule::setScheduleConfiguredTime(const QDateTime &aDateTime)
{
    d_ptr->iScheduleConfiguredTime = aDateTime;
}

QDateTime SyncSchedule::scheduleConfiguredTime()
{
    return d_ptr->iScheduleConfiguredTime;
}

QTime SyncSchedule::time() const
{
    return d_ptr->iTime;
}

void SyncSchedule::setTime(const QTime &aTime)
{
    d_ptr->iTime = aTime;
}

unsigned SyncSchedule::interval() const
{
    return d_ptr->iInterval;
}

void SyncSchedule::setInterval(unsigned aInterval)
{
    d_ptr->iInterval = aInterval;
}

bool SyncSchedule::scheduleEnabled() const
{
    return d_ptr->iEnabled;
}

void SyncSchedule::setScheduleEnabled(bool aEnabled)
{
    d_ptr->iEnabled = aEnabled;
}

bool SyncSchedule::rushEnabled() const
{
    return d_ptr->iRushEnabled;
}

void SyncSchedule::setRushEnabled(bool aEnabled)
{
    d_ptr->iRushEnabled = aEnabled;
}

bool SyncSchedule::syncExternallyDuringRush() const
{
    return d_ptr->iExternalRushEnabled;
}

void SyncSchedule::setSyncExternallyDuringRush(bool aEnabled)
{
    d_ptr->iExternalRushEnabled = aEnabled;
}

DaySet SyncSchedule::rushDays() const
{
    return d_ptr->iRushDays;
}

void SyncSchedule::setRushDays(const DaySet &aDays)
{
    d_ptr->iRushDays = aDays;
}

QTime SyncSchedule::rushBegin() const
{
    return d_ptr->iRushBegin;
}

QTime SyncSchedule::rushEnd() const
{
    return d_ptr->iRushEnd;
}

void SyncSchedule::setRushTime(const QTime &aBegin, const QTime &aEnd)
{
    d_ptr->iRushBegin = aBegin;
    d_ptr->iRushEnd = aEnd;
}

unsigned SyncSchedule::rushInterval() const
{
    return d_ptr->iRushInterval;
}

void SyncSchedule::setRushInterval(unsigned aInterval)
{
    d_ptr->iRushInterval = aInterval;
}

bool SyncSchedule::inExternalSyncRushPeriod(const QDateTime &aDateTime) const
{
    if (d_ptr->iEnabled && d_ptr->iRushEnabled && d_ptr->iExternalRushEnabled) {
        if (d_ptr->isRush(aDateTime)) {
            return true;
        }
    }
    return false;
}

QDateTime SyncSchedule::nextSyncTime(const QDateTime &aPrevSync) const                                 
{
    QDateTime nextSync;
    QDateTime scheduleConfiguredTime = d_ptr->iScheduleConfiguredTime;
    QDateTime now = QDateTime::currentDateTime();

    LOG_DEBUG("aPrevSync" << aPrevSync.toString() << "Last Configured Time " << scheduleConfiguredTime.toString()
              <<"CurrentDateTime"<<now);

    if (d_ptr->iTime.isValid() && !d_ptr->iDays.isEmpty())
    {
        // The sync time is defined explicitly (for ex. every Mon, Wed, at
        // 5:30PM). So choose the next applicable day from now.
    	LOG_DEBUG("Explicit sync time defined.");
        nextSync.setTime(d_ptr->iTime);
        nextSync.setDate(now.date());
        if (now.time() > d_ptr->iTime)
        {
            nextSync = nextSync.addDays(1);
        } // no else
        d_ptr->adjustDate(nextSync, d_ptr->iDays);
    }
    else if (d_ptr->iInterval > 0)
    {
        // Sync time is defined in terms of interval (for ex. every 15 minutes)
    	LOG_DEBUG("Sync interval defined as" << d_ptr->iInterval);
        // Last sync time is not available/valid (Could happen if the device
        // is shut down for an extended period before the first sync can be
        // performed). Hence use the time the
        // sync was last scheduled as the reference
        // Figure out the number of intervals passed
        QDateTime reference;
        reference = (aPrevSync.isValid()) ? aPrevSync : scheduleConfiguredTime;
        if(reference > now)
        {
            // If the clock was rolled back...
            LOG_DEBUG("Setting reference to now");
            reference = now;
        }
        else if (!reference.isValid()) {
           //It means configuring first time account. Need to sync now only.
           LOG_DEBUG("Reference is not valid returning current date time");
           return QDateTime::currentDateTime();
        }
        int numberOfIntervals = 0;
        if(0 != d_ptr->iInterval && d_ptr->iEnabled)
        {
            int secs = reference.secsTo(now) + 1;
            numberOfIntervals = secs/(d_ptr->iInterval * 60);
            if(secs % (d_ptr->iInterval * 60))
            {
                numberOfIntervals++;
            }
            LOG_DEBUG("numberOfInterval:"<<numberOfIntervals<<"interval time"<<d_ptr->iInterval);
        }
        nextSync = d_ptr->iEnabled ? reference.addSecs(numberOfIntervals * d_ptr->iInterval * 60) : QDateTime();
    }

    LOG_DEBUG("next non rush hour sync is at:: " << nextSync);

    // Rush is controlled by a external process(e.g always-up-to-date), buteo controls the switch between rush and offRush
    if (d_ptr->iRushEnabled && d_ptr->iExternalRushEnabled)
    {
        LOG_DEBUG("Rush Interval is controlled by a external process.");
        // Set next sync to rush end
        const bool isRush(d_ptr->isRush(now));
        QDateTime nextSyncRush;
        nextSyncRush.setTime(isRush ? d_ptr->iRushEnd : d_ptr->iRushBegin);
        nextSyncRush.setDate(now.date());
        if (now.time() > d_ptr->iRushEnd)
        {
            nextSyncRush = nextSyncRush.addDays(1);
        }
        d_ptr->adjustDate(nextSyncRush, d_ptr->iRushDays);
        LOG_DEBUG("Rush controlled by external process, next scheduled sync at rush " << (isRush ? "end" : "begin") << nextSyncRush.toString());
        // Use next sync time calculated with rush settings if necessary.
        if (nextSyncRush.isValid()) {
            // check to see if we should use it, or instead use the next non-rush sync time.
            if (nextSync.isValid()
                    && nextSync > now
                    && nextSync < nextSyncRush
                    && (!d_ptr->isRush(nextSync))) {
                // the next non-rush sync time occurs after now
                // but before the next rush sync time, and either it
                // doesn't fall within the rush period itself or the
                // next rush sync time is in the next rush period.
                // we should use the non-rush schedule.
                LOG_DEBUG("Using non-rush time as the next sync time");
            } else {
                nextSync = nextSyncRush;
            }
        }
    }
    else if (d_ptr->iRushEnabled && d_ptr->iRushInterval > 0 && !d_ptr->iExternalRushEnabled)
    {
        LOG_DEBUG("Calculating next sync time with rush settings. Rush Interval is " << d_ptr->iRushInterval);
        // Calculate next sync time with rush settings.
        QDateTime nextSyncRush;
        bool nextSyncRushInNextRushPeriod = false;
        if (d_ptr->isRush(now))
        {
            LOG_DEBUG("Current time is in rush");
            // We are in rush hour
            if(aPrevSync.isValid())
            {
                LOG_DEBUG("PrevSync is valid and isRush true.. ");
                nextSyncRush = aPrevSync.addSecs(d_ptr->iRushInterval * 60);
                if ((nextSyncRush < now) || (aPrevSync > now))
                {
                    // Use current time if the previous sync time is too old, or
                    // the clock has been rolled back
                    nextSyncRush = now.addSecs(d_ptr->iRushInterval * 60);
                    LOG_DEBUG("nextsyncRush based on aPrevSync"<<nextSyncRush);
                }
            }
            else
            {
                nextSyncRush = now.addSecs(d_ptr->iRushInterval * 60);
            }
            
            if (!d_ptr->isRush(nextSyncRush))
            {
                // If the calculated rush time does not lie in the rush
                // interval, choose the next available rush time as the begin
                // time for the rush interval
            	LOG_DEBUG("isRush False");
                nextSyncRushInNextRushPeriod = true;
                nextSyncRush.setTime(d_ptr->iRushBegin);
                if (nextSyncRush < now)
                {
                    nextSyncRush = nextSyncRush.addDays(1);
                } // no else
                d_ptr->adjustDate(nextSyncRush, d_ptr->iRushDays);
            } // no else
        }
        else
        {
        	LOG_DEBUG("Current Time is Not Rush");
            nextSyncRush.setTime(d_ptr->iRushBegin);
            nextSyncRush.setDate(now.date());
            if (now.time() > d_ptr->iRushBegin)
            {
                nextSyncRush = nextSyncRush.addDays(1);
            } // no else
            d_ptr->adjustDate(nextSyncRush, d_ptr->iRushDays);
        }

        LOG_DEBUG("nextSyncRush" << nextSyncRush.toString());
        // Use next sync time calculated with rush settings if necessary.
        if (nextSyncRush.isValid()) {
            // check to see if we should use it, or instead use the next non-rush sync time.
            if (nextSync.isValid()
                    && nextSync > now
                    && nextSync < nextSyncRush
                    && (!d_ptr->isRush(nextSync) || nextSyncRushInNextRushPeriod)) {
                // the next non-rush sync time occurs after now
                // but before the next rush sync time, and either it
                // doesn't fall within the rush period itself or the
                // next rush sync time is in the next rush period.
                // we should use the non-rush schedule.
                LOG_DEBUG("Using non-rush time as the next sync time");
            } else {
                // we should use the rush schedule.
                LOG_DEBUG("Using rush time as the next sync time");
                nextSync = nextSyncRush;
            }
        } // no else
    } // no else

    //For safer side checking nextSyncTime should not be behind currentDateTime.
    if ( QDateTime::currentDateTime().secsTo(nextSync) < 0 ) {
        //If it is the case making it to currentTime.
        LOG_WARNING("Something went wrong in nextSyncTime calculation resetting to current time");
        nextSync = QDateTime::currentDateTime();
    }

    LOG_DEBUG("nextSync" << nextSync.toString());
    return nextSync;
}

QDateTime SyncSchedule::nextRushSwitchTime(const QDateTime &aFromTime) const
{
    if (rushEnabled() && scheduleEnabled()) {
        if (d_ptr->iRushInterval == d_ptr->iInterval && !d_ptr->iExternalRushEnabled) {
            LOG_DEBUG("Rush interval is the same as normal interval no need to switch");
            return QDateTime();
        }
        if (d_ptr->isRush(aFromTime)) {
            return QDateTime(aFromTime.date(), d_ptr->iRushEnd);
        } else {
            // If rush day and before rush end next switch is at rush begin
            if (d_ptr->iRushDays.contains(aFromTime.date().dayOfWeek()) && aFromTime.time() < d_ptr->iRushBegin) {
                return QDateTime(aFromTime.date(), d_ptr->iRushBegin);
            } else {
                // Not a rush day or the rush period has ended, attemp switch at next day rush begin,
                // we can only schedule for 24h
                return QDateTime(aFromTime.date().addDays(1), d_ptr->iRushBegin);
            }
        }
    } else {
        return QDateTime();
    }
}


DaySet SyncSchedulePrivate::parseDays(const QString &aDays) const
{
    DaySet daySet;
    if (!aDays.isNull())
    {
        QStringList dayList = aDays.split(DAY_SEPARATOR,
            QString::SkipEmptyParts);
        foreach (QString dayStr, dayList)
        {
            bool ok;
            int dayNum = dayStr.toInt(&ok);
            if (ok)
            {
                daySet.insert(dayNum);
            } // no else
        }
    } // no else

    return daySet;
}

QString SyncSchedulePrivate::createDays(const DaySet &aDays) const
{
    QStringList dayList;

    foreach (int dayNum, aDays)
    {
        dayList.append(QString::number(dayNum));
    }

    return dayList.join(DAY_SEPARATOR);
}

bool SyncSchedulePrivate::adjustDate(QDateTime &aTime, const DaySet &aDays) const
{
    if (aDays.isEmpty())
    {
        aTime = QDateTime();
        return false;
    } // no else

    bool newValidDay = false;
    int startDay = aTime.date().dayOfWeek();
    while (!aDays.contains(aTime.date().dayOfWeek()))
    {
        newValidDay = true;
        aTime = aTime.addDays(1);
        // Safety check, avoid infinite loop if date set contains
        // only invalid values.
        if (aTime.date().dayOfWeek() == startDay)
        {
            // Clear next sync time.
            newValidDay = false;
            aTime = QDateTime();
            break;
        } // no else
    }

    return newValidDay;
}

bool SyncSchedulePrivate::isRush(const QDateTime &aTime) const
{
    return (iRushDays.contains(aTime.date().dayOfWeek()) &&
            aTime.time() >= iRushBegin && aTime.time() < iRushEnd);
}


