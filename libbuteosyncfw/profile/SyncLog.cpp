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
#include "SyncLog.h"
#include "LogMacros.h"
#include <QDomDocument>
#include <QtAlgorithms>

#include "ProfileEngineDefs.h"


namespace Buteo {

bool syncResultPointerLessThan(const SyncResults *&aLhs, const SyncResults *&aRhs)
{
    if (aLhs && aRhs) {
        return (*aLhs < *aRhs);
    }
    else {
        return false;
    }
}
    
// Private implementation class for SyncLog.
class SyncLogPrivate
{
public:
    SyncLogPrivate();

    SyncLogPrivate(const SyncLogPrivate &aSource);

    ~SyncLogPrivate();

    // Name of the profile this log belongs to.
    QString iProfileName;

    // List of the sync results this log consists of.
    QList<const SyncResults*> iResults;
};

}

using namespace Buteo;

SyncLogPrivate::SyncLogPrivate()
{
}

SyncLogPrivate::SyncLogPrivate(const SyncLogPrivate &aSource)
:   iProfileName(aSource.iProfileName)
{
    foreach (const SyncResults *results, aSource.iResults) {
        iResults.append(new SyncResults(*results));
    }
}

SyncLogPrivate::~SyncLogPrivate()
{
    qDeleteAll(iResults);
    iResults.clear();
}

SyncLog::SyncLog(const QString &aProfileName)
:   d_ptr(new SyncLogPrivate())
{
    d_ptr->iProfileName = aProfileName;
}

SyncLog::SyncLog(const QDomElement &aRoot)
:   d_ptr(new SyncLogPrivate())
{
    d_ptr->iProfileName = aRoot.attribute(ATTR_NAME);

    QDomElement results = aRoot.firstChildElement(TAG_SYNC_RESULTS);
    for (; !results.isNull();
         results = results.nextSiblingElement(TAG_SYNC_RESULTS)) {
        d_ptr->iResults.append(new SyncResults(results));
    }

    // Sort result entries by sync time.
    //qSort(d_ptr->iResults.begin(), d_ptr->iResults.end(), syncResultPointerLessThan);
}

SyncLog::SyncLog(const SyncLog &aSource)
:   d_ptr(new SyncLogPrivate(*aSource.d_ptr))
{
}

SyncLog::~SyncLog()
{
    delete d_ptr;
    d_ptr = 0;
}

void SyncLog::setProfileName(const QString& aProfileName)
{
    d_ptr->iProfileName = aProfileName;
}


QString SyncLog::profileName() const
{
    return d_ptr->iProfileName;
}

QDomElement SyncLog::toXml(QDomDocument &aDoc) const
{
    QDomElement root = aDoc.createElement(TAG_SYNC_LOG);
    root.setAttribute(ATTR_NAME, d_ptr->iProfileName);

    foreach (const SyncResults *results, d_ptr->iResults) {
        root.appendChild(results->toXml(aDoc));
    }

    return root;
}

const SyncResults* SyncLog::lastResults() const
{
    FUNCTION_CALL_TRACE;
    if (d_ptr->iResults.isEmpty()) {
        return 0;
    }
    else {
        return d_ptr->iResults.last();
    }
}

QList<const SyncResults*> SyncLog::allResults() const
{
    return d_ptr->iResults;
}

void SyncLog::addResults(const SyncResults &aResults)
{
    FUNCTION_CALL_TRACE;
    // To prevent the log growing too much, the maximum number of entries in 
    //the log is defined
    const int MAXLOGENTRIES = 5;
    
    if (d_ptr->iResults.size() == MAXLOGENTRIES) {
        // The list is sorted so that the oldest item is in the beginning
        delete d_ptr->iResults.takeFirst();
    } // no else
    
    d_ptr->iResults.append(new SyncResults(aResults));

    // Sort result entries by sync time.
    //qSort(d_ptr->iResults.begin(), d_ptr->iResults.end(), syncResultPointerLessThan);
}
