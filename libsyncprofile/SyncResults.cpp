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


#include "SyncResults.h"
#include "LogMacros.h"
#include <QDomDocument>

#include "ProfileEngineDefs.h"


namespace Buteo {
    
// Private implementation class for SyncResults.
class SyncResultsPrivate
{
public:
    SyncResultsPrivate();

    SyncResultsPrivate(const SyncResultsPrivate &aSource);

    // List of target results.
    QList<TargetResults> iTargetResults;

    // Sync time.
    QDateTime iTime;

    // Sync result code.
    int iResultCode;

    //Sync target id
    QString iTargetId;

    bool iScheduled;
};

}

using namespace Buteo;

SyncResultsPrivate::SyncResultsPrivate()
:   iTime(QDateTime::currentDateTime()),
    iResultCode(0),
    iScheduled(false)
{
}

SyncResultsPrivate::SyncResultsPrivate(const SyncResultsPrivate &aSource)
:   iTargetResults(aSource.iTargetResults),
    iTime(aSource.iTime),
    iResultCode(aSource.iResultCode),
    iScheduled(aSource.iScheduled),
    iTargetId(aSource.iTargetId)
{
}

SyncResults::SyncResults()
:   d_ptr(new SyncResultsPrivate())
{
}

SyncResults::SyncResults(const SyncResults &aSource)
:   d_ptr(new SyncResultsPrivate(*aSource.d_ptr))
{
}

SyncResults::SyncResults(QDateTime aTime, int aResultCode)
:   d_ptr(new SyncResultsPrivate())
{
    d_ptr->iTime = aTime;
    d_ptr->iResultCode = aResultCode;
}

SyncResults::SyncResults(const QDomElement &aRoot)
:   d_ptr(new SyncResultsPrivate())
{
    d_ptr->iTime = QDateTime::fromString(aRoot.attribute(ATTR_TIME), Qt::ISODate);
    d_ptr->iResultCode = aRoot.attribute(ATTR_RESULT_CODE).toInt();
    d_ptr->iScheduled = (aRoot.attribute(KEY_SYNC_SCHEDULED) == BOOLEAN_TRUE);

    QDomElement target = aRoot.firstChildElement(TAG_TARGET_RESULTS);
    for (; !target.isNull();
         target = target.nextSiblingElement(TAG_TARGET_RESULTS))
    {
        d_ptr->iTargetResults.append(TargetResults(target));
    }
}

SyncResults::~SyncResults()
{
    delete d_ptr;
    d_ptr = 0;
}

SyncResults& SyncResults::operator=(const SyncResults &aRhs)
{
    if (&aRhs != this)
    {
        delete d_ptr;
        d_ptr = new SyncResultsPrivate(*aRhs.d_ptr);
    }

    return *this;
}

QDomElement SyncResults::toXml(QDomDocument &aDoc) const
{
    QDomElement root = aDoc.createElement(TAG_SYNC_RESULTS);
    root.setAttribute(ATTR_TIME, d_ptr->iTime.toString(Qt::ISODate));
    root.setAttribute(ATTR_RESULT_CODE, QString::number(d_ptr->iResultCode));
    root.setAttribute(KEY_SYNC_SCHEDULED, d_ptr->iScheduled ? BOOLEAN_TRUE :
        BOOLEAN_FALSE);

    foreach (TargetResults tr, d_ptr->iTargetResults)
    {
        root.appendChild(tr.toXml(aDoc));
    }

    return root;
}

QList<TargetResults> SyncResults::targetResults() const
{
    return d_ptr->iTargetResults;
}

void SyncResults::addTargetResults(const TargetResults &aResults)
{
    d_ptr->iTargetResults.append(aResults);
}

QDateTime SyncResults::syncTime() const
{
    return d_ptr->iTime;
}

int SyncResults::resultCode() const
{
    return d_ptr->iResultCode;
}

void SyncResults::setResultCode(int aResultCode)
{
  FUNCTION_CALL_TRACE;
    d_ptr->iResultCode = aResultCode;
}

void SyncResults::setTargetId(const QString& aTargetId) 
{
    d_ptr->iTargetId = aTargetId;
}

QString SyncResults::getTargetId() const
{
    return d_ptr->iTargetId;
}

bool SyncResults::operator<(const SyncResults &aOther) const
{
    return (d_ptr->iTime < aOther.d_ptr->iTime);
}

void SyncResults::setScheduled(bool aScheduled)
{
    d_ptr->iScheduled = aScheduled;
}

bool SyncResults::isScheduled() const
{
    return d_ptr->iScheduled;
}
